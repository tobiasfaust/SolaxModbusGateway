#include "modbus.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : enableRelays(false), Baudrate(19200), enableCrcCheck(true), enableLengthCheck(true), LastTxLiveData(0), LastTxIdData(0), LastTxInverter(0) {
  DataFrame           = new std::vector<byte>{};
  SaveIdDataframe     = new std::vector<byte>{};
  SaveLiveDataframe   = new std::vector<byte>{};

  InverterLiveData    = new std::vector<reg_t>{};
  InverterIdData      = new std::vector<reg_t>{};
  AvailableInverters  = new std::vector<regfiles_t>{};
  Setters             = new std::vector<subscription_t>{};

  Conf_RequestLiveData= new std::vector<std::vector<byte>>{};
  Conf_RequestIdData  = new std::vector<byte>{};

  InverterType        = {};

  ReadQueue = new ArduinoQueue<std::vector<byte>>(5); // max 5 read requests parallel
  SetQueue  = new ArduinoQueue<std::vector<byte>>(5); // max 5 set requests parallel

  if (Config->GetUseETH()) {
    this->pin_RX = this->default_pin_RX = 2;
    this->pin_TX = this->default_pin_TX = 4;
    this->pin_RTS = this->default_pin_RTS = 5;
    this->pin_Relay1 = this->default_pin_Relay1 = 12;
    this->pin_Relay2 = this->default_pin_Relay2 = 14;
  } else {
    this->pin_RX = this->default_pin_RX = 16;
    this->pin_TX = this->default_pin_TX = 17;
    this->pin_RTS = this->default_pin_RTS = 5;
    this->pin_Relay1 = this->default_pin_Relay1 = 18;
    this->pin_Relay2 = this->default_pin_Relay2 = 19;
  }

  this->LoadInvertersFromJson(); //needed for selecting default inverter
  this->LoadJsonConfig(true);
  this->init(true);
}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init(bool firstrun) {
  if (Config->GetDebugLevel() >=3) {
    dbg.printf("Start Hardwareserial 1 on RX (%d), TX(%d), RTS(%d)\n", this->pin_RX, this->pin_TX, this->pin_RTS);
    dbg.printf("Init Modbus to Client 0x%02X with %d Baud\n", this->ClientID, this->Baudrate);

  }

  // Configure Direction Control pin
  pinMode(this->pin_RTS, OUTPUT);  
  if (this->enableRelays) {
    pinMode(this->pin_Relay1, INPUT_PULLUP);
    pinMode(this->pin_Relay2, INPUT_PULLUP);
  }

  this->LoadInvertersFromJson();
  this->LoadInverterConfigFromJson();
  this->LoadRegItems(this->InverterIdData, "id");
  this->LoadRegItems(this->InverterLiveData, "livedata");
  this->LoadJsonItemConfig(); // loads InverterLiveData Items too
  

  // https://forum.arduino.cc/t/creating-serial-objects-within-a-library/697780/11
  RS485Serial = new HardwareSerial(1);
  RS485Serial->begin(this->Baudrate, SERIAL_8N1, this->pin_RX, this->pin_TX);

  //at first read ID Data
  this->QueryIdData();
}

/*******************************************************
 * Read configured pin states
*******************************************************/
void modbus::ReadRelays() {
  this->state_Relay1 = digitalRead(this->pin_Relay1);    
  this->mqtt->Publish_Int("relay1", this->state_Relay1, false);
  
  this->state_Relay2 = digitalRead(this->pin_Relay2);    
  this->mqtt->Publish_Int("relay2", this->state_Relay2, false);
}

/*******************************************************
 * generate the full mqtt topic without /# at end
*******************************************************/
String modbus::GetMqttSetTopic(String command) {
  char s[100] = {0}; 
  memset(s, 0, sizeof(s));

  snprintf(s, sizeof(dbg), "%s/%s/set/%s", Config->GetMqttBasePath().c_str(), Config->GetMqttRoot().c_str(), command.c_str());
  return (String)s;
}

/*******************************************************
 * subscribe to all possible "set" register (register.h)
*******************************************************/
void modbus::GenerateMqttSubscriptions() {
  // clear vector
  this->Setters->clear();
  
  File regfile = LittleFS.open("/regs/"+this->InverterType.filename);
  
  String streamString = "";
  streamString = "\""+ this->InverterType.name +"\": {";
  regfile.find(streamString.c_str());
  streamString = "\"set\": [";
  regfile.find(streamString.c_str());
  do {
    JsonDocument elem;
    DeserializationError error = deserializeJson(elem, regfile); 
    if (!error) {
      // Print the result
      if (Config->GetDebugLevel() >=4) {dbg.println("parsing JSON for <set> data ok"); }
      if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);}
     
      if(!elem["name"].isNull() && elem["request"].is<JsonArray>()) {
        subscription_t s = {};
        s.command = elem["name"].as<String>();  
        
        JsonArray arr = elem["request"].as<JsonArray>();
        std::vector<byte> t = {};
        for (String x : arr) {
          byte e = this->String2Byte(x);
          t.push_back(e);
        }
        s.request = t;        

        this->mqtt->Subscribe(this->GetMqttSetTopic(s.command));
        if (Config->GetDebugLevel() >=4) {
          dbg.printf("Set command successfully parsed from JSON: %s with %s\n", s.command.c_str(), (this->PrintDataFrame(&(s.request))).c_str());        }
        this->Setters->push_back(s);

      } else {
        continue;
      }
     
    } else {
      if (Config->GetDebugLevel() >=1) {
        dbg.printf("Failed to parse JSON Register <set> Data: %s\n", error.c_str());
      }
    }
  } while (regfile.findUntil(",","]"));

  regfile.close();
}


/*******************************************************
 * act on received mqtt command
*******************************************************/
void modbus::ReceiveMQTT(String topic, int msg) {
  if (!this->Conf_EnableSetters) {
    if (Config->GetDebugLevel() >=2) {
      dbg.printf("Set command <%s> received, but setters over mqtt are currently disabled\n", topic.c_str());
    }
    return;
  }

  for (uint8_t i = 0; i < this->Setters->size(); i++ ) {
    if (topic == this->GetMqttSetTopic(this->Setters->at(i).command)) {
      std::vector<byte> request = this->Setters->at(i).request;
      byte bytes[4];

      bytes[0] = (msg >> 24) & 0xFF;
      bytes[1] = (msg >> 16) & 0xFF;
      bytes[2] = (msg >> 8) & 0xFF;
      bytes[3] = (msg >> 0) & 0xFF;

      // 16bit number
      request.push_back(bytes[2]);
      request.push_back(bytes[3]);

      if (Config->GetDebugLevel() >=3) {
        dbg.printf("MQTT Setter found: %s\n" ,this->Setters->at(i).command.c_str());
        dbg.printf("Initiate Set Request to queue: %s\n" ,(this->PrintDataFrame(&request)).c_str());
      }

      this->SetQueue->enqueue(request);
    }
  }
}

/*******************************************************
 * get all defined inverters from json (register.h)
*******************************************************/
void modbus::LoadInvertersFromJson() {
  JsonDocument regjson;
  JsonDocument filter;

  AvailableInverters->clear();

  filter["*"]["config"]["ClientIdPos"] = true;  
  File root = LittleFS.open("/regs/");
  File file = root.openNextFile();
  while(file){
    if (Config->GetDebugLevel() >=3) { dbg.printf("open register file from Filesystem: %s\n", file.name()); }

    DeserializationError error = deserializeJson(regjson, file, DeserializationOption::Filter(filter));
     if (!error && regjson.size() > 0) {
      // https://arduinojson.org/v6/api/jsonobject/begin_end/
      JsonObject root = regjson.as<JsonObject>();
      for (JsonPair kv : root) {
        if (Config->GetDebugLevel() >=3) {
          dbg.printf("Inverter found: %s\n", kv.key().c_str());
        }

        regfiles_t wr = {};
        wr.filename = file.name();
        wr.name = kv.key().c_str();
        AvailableInverters->push_back(wr);
      }
    } else{
      dbg.printf("Error: unable to load inverters from File %s: %s\n", file.name(), error.c_str());
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();

  if (this->AvailableInverters->size() == 0) {
    dbg.println("ALERT: No register definitions found. ESP cannot work properly");
    dbg.println("Please flash filesystem Image!");
  }
}

/*******************************************************
 * read config data from JSON, part "config"
*******************************************************/
void modbus::LoadInverterConfigFromJson() {
  JsonDocument doc;
  JsonDocument filter;

  File regfile = LittleFS.open("/regs/"+this->InverterType.filename);
  if (!regfile) {
    dbg.println("failed to open ModbusConfig.json file for writing");
  }

  filter[this->InverterType.name]["config"] = true;
    
  DeserializationError error = deserializeJson(doc, regfile, DeserializationOption::Filter(filter));

  if (error && Config->GetDebugLevel() >=1) {
    dbg.printf("Error: unable to read configdata for inverter %s: %s\n", this->InverterType.name.c_str(), error.c_str());
  } else {
    if (Config->GetDebugLevel() >=4) {
      dbg.printf("Read config data for inverter %s\n", this->InverterType.name.c_str());
      serializeJsonPretty(doc, dbg);
      dbg.println();
    }
  }

  //this->Conf_LiveDataFunctionCode   = this->String2Byte(doc[this->InverterType.name]["config"]["LiveDataFunctionCode"].as<String>());
  //this->Conf_IdDataFunctionCode     = this->String2Byte(doc[this->InverterType.name]["config"]["IdDataFunctionCode"].as<String>());
  this->Conf_LiveDataErrorCode      = this->String2Byte(doc[this->InverterType.name]["config"]["LiveDataErrorCode"].as<String>());
	this->Conf_IdDataErrorCode        = this->String2Byte(doc[this->InverterType.name]["config"]["IdDataErrorCode"].as<String>());
	//this->Conf_LiveDataSuccessCode    = this->String2Byte(doc[this->InverterType.name]["config"]["LiveDataSuccessCode"].as<String>());
	//this->Conf_IdDataSuccessCode      = this->String2Byte(doc[this->InverterType.name]["config"]["IdDataSuccessCode"].as<String>());
  this->Conf_ClientIdPos            = int(doc[this->InverterType.name]["config"]["ClientIdPos"]);
  //this->Conf_LiveDataStartsAtPos    = int(doc[this->InverterType.name]["config"]["LiveDataStartsAtPos"]);
	//this->Conf_IdDataStartsAtPos      = int(doc[this->InverterType.name]["config"]["IdDataStartsAtPos"]);
	this->Conf_LiveDataErrorPos       = int(doc[this->InverterType.name]["config"]["LiveDataErrorPos"]);
	this->Conf_IdDataErrorPos         = int(doc[this->InverterType.name]["config"]["IdDataErrorPos"]);
	//this->Conf_LiveDataSuccessPos     = int(doc[this->InverterType.name]["config"]["LiveDataSuccessPos"]);
	//this->Conf_IdDataSuccessPos       = int(doc[this->InverterType.name]["config"]["IdDataSuccessPos"]);
  //this->Conf_IdDataFunctionCodePos  = int(doc[this->InverterType.name]["config"]["IdDataFunctionCodePos"]);
  //this->Conf_LiveDataFunctionCodePos= int(doc[this->InverterType.name]["config"]["LiveDataFunctionCodePos"]);
  
  Conf_RequestLiveData->clear();
  for (JsonArray arr : doc[this->InverterType.name]["config"]["RequestLiveData"].as<JsonArray>()) {
  
    std::vector<byte> t = {};
    for (String x : arr) {
      byte e = this->String2Byte(x);
      t.push_back(e);
    }
    t.push_back(DATAISLIVE); // last byte is datatype
    Conf_RequestLiveData->push_back(t);
  }
  
  Conf_RequestIdData->clear();
  for (String elem : doc[this->InverterType.name]["config"]["RequestIdData"].as<JsonArray>()) {
    byte e = this->String2Byte(elem);
    Conf_RequestIdData->push_back(e);
  }
  Conf_RequestIdData->push_back(DATAISID); // last byte is datatype

  if (regfile) { regfile.close(); }

}

/*******************************************************
 * convert a particular string from JSON to a byte
*******************************************************/
byte modbus::String2Byte(String s){
  byte ret = 0x00;
  if(s.startsWith("0x")) {
    ret = strtoul(s.substring(2).c_str(), NULL, 16);
  } else if(s.equalsIgnoreCase("#clientid")) {
    ret = this->ClientID;
  }
  return ret;
}

/*******************************************************
 * Enable MQTT Transmission
*******************************************************/
void modbus::enableMqtt(MQTT* object) {
  this->mqtt = object;
  dbg.println("MQTT aktiviert");

  this->GenerateMqttSubscriptions();
}

/*******************************************************
 * Query ID Data to Inverter
*******************************************************/
void modbus::QueryIdData() {
  if (Config->GetDebugLevel() >=4) {dbg.println("Query ID Data into Queue:");}
  
  /* byte message[] = {this->ClientID, 
                               0x03,  // FunctionCode
                               0x00,  // StartAddress MSB
                               0x00,  // StartAddress LSB
                               0x00,  // Anzahl Register MSB
                               0x14,  // Anzahl Register LSB
                               0x00,  // CRC LSB
                               0x00   // CRC MSB
           }; // 
  */

  if (this->ReadQueue->isEmpty()) {
    if (Config->GetDebugLevel() >=4) { dbg.println(this->PrintDataFrame(this->Conf_RequestIdData).c_str()); }
    this->ReadQueue->enqueue(*this->Conf_RequestIdData);
  }
}


/*******************************************************
 * Query Live Data to Inverter
*******************************************************/
void modbus::QueryLiveData() {
  if (Config->GetDebugLevel() >=4) {dbg.println("Query Live Data into Queue:"); }
   
  /* byte message[] = {this->ClientID, 
                               0x04,  // FunctionCode
                               0x00,  // StartAddress MSB
                               0x00,  // StartAddress LSB
                               0x00,  // Anzahl Register MSB
                               0x23,  // Anzahl Register LSB
                               0x00,  // CRC LSB
                               0x00   // CRC MSB
           }; // 
  */

  if (this->ReadQueue->isEmpty()) {
    for (uint8_t i = 0; i < this->Conf_RequestLiveData->size(); i++) {
      if (Config->GetDebugLevel() >=4) { dbg.println(this->PrintDataFrame(&this->Conf_RequestLiveData->at(i)).c_str()); }
      this->ReadQueue->enqueue(this->Conf_RequestLiveData->at(i));
    }
  }
}

/*******************************************************
 * send 1 Read query from queue to inverter 
*******************************************************/
void modbus::QueryQueueToInverter() {
  enum rwtype_t {READ, WRITE, NUL};
  rwtype_t rwtype;
  std::vector<byte> m = {};
  byte rememberdatatype = 0;

  if (!this->SetQueue->isEmpty()) {
    rwtype = WRITE;
    m = this->SetQueue->dequeue();
  } // writing has priority
  else if (!this->ReadQueue->isEmpty()) {
    rwtype = READ;
    m = this->ReadQueue->dequeue();
    rememberdatatype = m[m.size()-1];  // store current datatype
    m.pop_back();                      // shorten queue item to original size
  }
  else { rwtype = NUL; }

  if (rwtype != NUL) {
    if (Config->GetDebugLevel() >=3) { dbg.print("Request queue data to inverter: "); }
  
    digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
    while (RS485Serial->available() > 0) { // read serial if any old data is available
      RS485Serial->read();
    }

    byte message[m.size() + 2] = {0x00}; // +2 Byte CRC
  
    for (uint8_t i=0; i < m.size(); i++) {
      message[i] = m.at(i);
    }

    uint16_t crc = this->Calc_CRC(message, sizeof(message)-2);
    message[sizeof(message)-1] = highByte(crc);
    message[sizeof(message)-2] = lowByte(crc);
    m.push_back(lowByte(crc));
    m.push_back(highByte(crc));

    if (Config->GetDebugLevel() >=3) { dbg.println(this->PrintDataFrame(message, sizeof(message))); }

    digitalWrite(this->pin_RTS, RS485Transmit);     // init Transmit
    RS485Serial->write(message, sizeof(message));
    RS485Serial->flush();
    digitalWrite(this->pin_RTS, RS485Receive); 

    // wait 100ms to get response
    unsigned long timeout=millis()+100;
    while (millis()<=timeout) { yield(); }

    if (rwtype == WRITE) {
      this->ReceiveSetData(&m);
    } 
    else if (rwtype == READ) { 
      this->ReceiveReadData();
      if (this->ReadQueue->isEmpty()) {
        this->DataFrame->push_back(rememberdatatype);  // add datatype at the end of the received frame
        this->ParseData();
      }
    }

  }
}

/***********************************************************
 * Receive Data after a Set Query, Check if successful set
************************************************************/
bool modbus::ReceiveSetData(std::vector<byte>* SendHexFrame) {
  std::vector<byte> RecvHexframe = {};
  bool ret = false; 

  if (Config->GetDebugLevel() >=3) {dbg.println("Read Data from Queue: ");}

  digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
  if (RS485Serial->available()) {
    // lese alle Daten, speichern im Vektor "Dataframe"
    while(RS485Serial->available()) {
      byte d = RS485Serial->read();
      RecvHexframe.push_back(d);
      if (Config->GetDebugLevel() >=4) {dbg.print(PrintHex(d)); dbg.print(" ");}
      delay(1); // keep this! Loosing bytes possible if too fast
    }    
    if (Config->GetDebugLevel() >=4) {dbg.println();}
    
    // TODO
    // compare Set and Received Answer, should be equal
    //if (SendHexFrame == RecvHexframe) {
      // successful
      //ret = true;
    //}
  }

  return ret;
}

/*******************************************************
 * Receive Data after Quering, put them into vector
*******************************************************/
void modbus::ReceiveReadData() {
  size_t dataFrameStartPos =  this->DataFrame->size();

  if (Config->GetDebugLevel() >=3) {dbg.println("Read Data from Queue: ");}

  digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
  if (RS485Serial->available()) {
    // lese alle Daten, speichern im Vektor "Dataframe"
    while(RS485Serial->available()) {
      byte d = RS485Serial->read();
      this->DataFrame->push_back(d);
      if (Config->GetDebugLevel() >=4) {dbg.print(PrintHex(d)); dbg.print(" ");}
      delay(1); // keep this! Loosing bytes possible if too fast
    }
    if (Config->GetDebugLevel() >=4) {dbg.println();}
    
    bool valid = true;

    if (this->DataFrame->size()-dataFrameStartPos > 5 &&
        this->DataFrame->at(dataFrameStartPos+this->Conf_ClientIdPos) == this->ClientID && 
        this->DataFrame->at(dataFrameStartPos+this->Conf_IdDataErrorPos) != this->Conf_IdDataErrorCode && 
        this->DataFrame->at(dataFrameStartPos+this->Conf_LiveDataErrorPos) != this->Conf_LiveDataErrorCode) {

      if (Config->GetDebugLevel() >=4) dbg.print("ErrorCode passed, OK");
      
      if (this->enableCrcCheck) {
        //CRC Check
        uint16_t crc = this->Calc_CRC(this->DataFrame, dataFrameStartPos, this->DataFrame->size()-2);
        
        if (Config->GetDebugLevel() >=4) {
          dbg.printf("Received CRC: 0x%02X 0x%02X\n", this->DataFrame->at(this->DataFrame->size()-2), this->DataFrame->at(this->DataFrame->size()-1));
          dbg.printf("Calculated CRC: 0x%02X 0x%02X\n", lowByte(crc), highByte(crc));
        }

        if (this->DataFrame->at(this->DataFrame->size()-2) != lowByte(crc) ||
            this->DataFrame->at(this->DataFrame->size()-1) != highByte(crc)) {
          valid = false;
          if (Config->GetDebugLevel() >=2) dbg.println("CRC check  failed!");
        }
      }

      if (this->enableLengthCheck) {
        // Check datalength
        if (Config->GetDebugLevel() >=4) {
          dbg.printf("Dataframe length should be: %d, is: %d bytes\n", this->DataFrame->at(dataFrameStartPos+2), this->DataFrame->size()-dataFrameStartPos-5);
        }

        if (this->DataFrame->at(dataFrameStartPos+2) != this->DataFrame->size()-dataFrameStartPos-5) {
          valid = false;
          if (Config->GetDebugLevel() >=2) dbg.printf("data length check failed, should be %d but is %d bytes\n", this->DataFrame->at(dataFrameStartPos+2), this->DataFrame->size()-dataFrameStartPos-5);
        }
      } 
    } else { valid = false; } 
    
    if (valid) {
      // Dataframe valid
      if (Config->GetDebugLevel() >=3) {
        dbg.printf("Dataframe valid, Dateframe size: %d bytes\n", this->DataFrame->size());
      }

    } else {
      if (Config->GetDebugLevel() >=2) {dbg.println("Dataframe invalid");}
      // clear dataframe, clear ReadQueue to start from fresh
      this->DataFrame->clear();
      for (unsigned int n = 0; n < this->ReadQueue->itemCount(); n++) {
        this->ReadQueue->dequeue();
      }
    }

  } else {
    if (Config->GetDebugLevel() >=2) {dbg.println("no response from client");}
  }
}

/*******************************************************
 * Helper function
 * convert a json position array to integer values of Dataframe
*******************************************************/
int modbus::JsonPosArrayToInt(JsonArray posArray, JsonArray posArray2) {
  uint32_t val_i = 0;
  uint32_t val2_i = 0;
  uint32_t val_max = 0;

  // handle Main Position Array
  if (!posArray.isNull()){ 
    for(uint16_t v : posArray) {
      if (v < this->DataFrame->size()) { 
        val_i = (val_i << 8) | this->DataFrame->at(v); 
        val_max = (val_max << 8) | 0xFF;
      }
    }
  }

  // handle Position Array 2
  if (!posArray2.isNull()){ 
    for(uint16_t w : posArray2) {
      if (w < this->DataFrame->size()) { 
        val2_i = (val2_i << 8) | this->DataFrame->at(w); 
      }
    }
  }

  // Check Type 1: convert into a negative value

  if (posArray2.isNull() && (val_i > (val_max/2))) {
     // negative Zahl, umrechnen notwendig 
    val_i = val_i ^ val_max;
    val_i = val_i * -1;
  } 

  // Check type 2: convert into a negative value
  // an extra position field for a neg value existis. Take this if value > 0
  if (val2_i > 0) {
    val_i = val2_i * -1;
  } 

  return val_i;
}

/*******************************************************
 * Parse all received Data in vector 
*******************************************************/
void modbus::ParseData() {
  char buffer[100] = {0}; 
  memset(buffer, 0, sizeof(buffer));
  
  String RequestType = "";
  byte tempbyte;

  if (this->DataFrame->size() <= 1) {

    // TODO
    //  ***********************************************
    // do some tests if client isn´t connected, dataframe is empty
    //  ***********************************************
    #ifdef DEBUGMODE 
      this->DataFrame->clear();
      if (Config->GetDebugLevel() >=3) {dbg.println("Start parsing in testmode, use some testdata instead real live data :)");}
      
      // Solar-KTL 
      //byte ReadBuffer[] = {0x01, 0x03, 0x60, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x34, 0x01, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x13, 0x86, 0x09, 0x11, 0x01, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4C, 0x00, 0x00, 0x02, 0x5F, 0x00, 0x8A, 0x01, 0x7D, 0x00, 0x28, 0x00, 0x33, 0x0E, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x60, 0x01, 0x42, 0x0A, 0x3B, 0x00, 0x0E, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x25};

      // Solax MIC
      //byte ReadBuffer[] = {0x01, 0x04, 0x80, 0x12, 0x34, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x09, 0x64, 0x09, 0x67, 0x09, 0x6B, 0x13, 0x8C, 0x13, 0x8D, 0x13, 0x8B, 0x00, 0x27, 0x00, 0x28, 0x00, 0x28, 0x00, 0x2C, 0x0B, 0x54, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x70, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5B, 0xC3};
      
      //Solax X1
      //byte ReadBuffer[] = {0x01,0x04,0xEE,0x09,0x29,0x00,0x5E,0x08,0x9E,0x0B,0xFA,0x0B,0x44,0x00,0x16,0x00,0x39,0x13,0x8A,0x00,0x26,0x00,0x02,0x02,0xB8,0x06,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0xB6,0x00,0x00,0x03,0xAC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x6E,0xFB,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xB9,0xA4,0x01,0x04,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x59,0x77,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x69,0x00,0x00,0x86,0x6B,0x00,0x01,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x70,0x00,0x00,0x03,0xF2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0xB9,0x02};
      //byte ReadBuffer[] = {0x01,0x04,0xEE,0x09,0x29,0x00,0x5E,0x08,0x9E,0x0B,0xFA,0x0B,0x44,0x00,0x16,0x00,0x39,0x13,0x8A,0x00,0x26,0x00,0x02,0x02,0xB8,0x06,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0xB6,0x00,0x00,0x03,0xAC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x6E,0xFB,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xB9,0xA4,0x02};
      byte ReadBuffer[] = {0x01,0x03,0x28,0x48,0x34,0x35,0x30,0x32,0x41,0x49,0x34,0x34,0x35,0x39,0x30,0x30,0x35,0x73,0x6F,0x6C,0x61,0x78,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x4A,0xA0, 0x01};
      
      //Growatt IDData
      //byte ReadBuffer[] = {0x01,0x03,0xEE,0x00,0x01,0x00,0xBD,0xFF,0xFF,0x00,0x64,0x00,0x00,0x27,0x10,0x00,0x00,0x9C,0x40,0x06,0x40,0x44,0x4E,0x31,0x2E,0x30,0x00,0x5A,0x42,0x44,0x42,0x00,0x02,0x00,0x02,0x00,0x00,0x06,0x40,0x00,0x3C,0x00,0x3C,0x00,0x5A,0x00,0x5A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x64,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x50,0x56,0x20,0x49,0x6E,0x76,0x65,0x72,0x74,0x65,0x72,0x20,0x20,0x00,0x00,0x15,0x18,0x02,0x03,0x07,0xE8,0x00,0x07,0x00,0x13,0x00,0x08,0x00,0x10,0x00,0x05,0x00,0x05,0x0C,0x73,0x13,0x73,0x12,0x8E,0x14,0x1E,0x07,0x00,0x13,0x73,0x12,0x8E,0x14,0x1E,0x07,0x00,0x13,0x73,0x12,0x8E,0x14,0x1E,0x0D,0x61,0x10,0xF6,0x12,0x9D,0x13,0x8D,0x00,0x98,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x00,0x32,0x11,0x1E,0x00,0x00,0x44,0x4E,0x41,0x41,0x30,0x31,0x35,0x31,0x30,0x30,0x30,0x32,0x01,0x31,0x00,0x00,0x00,0x00,0x13,0x9C,0x00,0x32,0x10,0x07,0x10,0xA6,0x0F,0x18,0x0E,0x79,0x00,0x14,0x00,0x05,0x10,0x57,0x0F,0x90,0x26,0xFD,0x26,0xFD,0x26,0xA5,0x27,0xC1,0x27,0xC1,0x28,0x21,0x00,0x0A,0x00,0x00,0x01,0xE4,0x00,0xFF,0x4E,0x20,0x00,0xFF,0x4E,0x20,0x00,0xFF,0x4E,0x20,0x00,0xFF,0x4E,0x20,0x07,0x00,0xFC,0xE5};
      
      //Growatt LiveData
      //byte ReadBuffer[] = {0x01,0x04,0xEE,0x00,0x01,0x00,0x00,0x32,0x8F,0x07,0xF5,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0xC2,0x00,0x2B,0x00,0x00,0x32,0x8F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0xED,0x13,0x86,0x09,0x5B,0x00,0x06,0x00,0x00,0x05,0x9D,0x09,0x5C,0x00,0x06,0x00,0x00,0x05,0x9D,0x09,0x4D,0x00,0x06,0x00,0x00,0x05,0x94,0x10,0x54,0x10,0x1B,0x10,0x1B,0x00,0x00,0x00,0x03,0x00,0x00,0x5F,0x9F,0x01,0xA8,0xF8,0x7A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x67,0x99,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x99,0x01,0xBD,0x01,0x63,0x01,0x54,0x00,0x00,0x01,0x79,0x0C,0xD4,0x0C,0xCA,0x4E,0x20,0x00,0x00,0x00,0x00,0x9C,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFA,0x01,0x04,0xEE,0x4E,0x20,0x00,0x03,0x00,0x00,0x9C,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x25,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x02,0x00,0x00,0x20,0x19,0x00,0x00,0x00,0x08,0x00,0x00,0x21,0xD5,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x51,0x00,0x00,0x66,0xD9,0x00,0x00,0x00,0x05,0x00,0x00,0x50,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x10,0x00,0x00,0x19,0xA0,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x53,0x00,0x00,0x30,0x00,0x12,0x19,0x9E,0x0D,0x4A,0x00,0x15,0x00,0x15,0x01,0x61,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x14,0x00,0x00,0x20,0x19,0x00,0x00,0x21,0xD5,0x00,0x00,0x00,0x03,0x0C,0x54,0x00,0x39,0x00,0x10,0x00,0x00,0x01,0x01,0x00,0xE1,0x00,0x02,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x31,0x00,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x12,0x52,0x94,0x01,0xCC,0x01,0xA5,0xD1,0x6F,0xEE,0x00,0x01,0x00,0x00,0x32,0x8F,0x07,0xF5,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0xC2,0x00,0x2B,0x00,0x00,0x32,0x8F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0xED,0x13,0x86,0x09,0x5B,0x00,0x06,0x00,0x00,0x05,0x9D,0x09,0x5C,0x00,0x06,0x00,0x00,0x05,0x9D,0x09,0x4D,0x00,0x06,0x00,0x00,0x05,0x94,0x10,0x54,0x10,0x1B,0x10,0x1B,0x00,0x00,0x00,0x03,0x00,0x00,0x5F,0x9F,0x01,0xA8,0xF8,0x7A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x67,0x99,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x99,0x01,0xBD,0x01,0x63,0x01,0x54,0x00,0x00,0x01,0x79,0x0C,0xD4,0x0C,0xCA,0x4E,0x20,0x00,0x00,0x00,0x00,0x9C,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFA,0x01,0x04,0xEE,0x4E,0x20,0x00,0x03,0x00,0x00,0x9C,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x25,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x02,0x00,0x00,0x20,0x19,0x00,0x00,0x00,0x08,0x00,0x00,0x21,0xD5,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x51,0x00,0x00,0x66,0xD9,0x00,0x00,0x00,0x05,0x00,0x00,0x50,0x16,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x10,0x00,0x00,0x19,0xA0,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x53,0x00,0x00,0x30,0x00,0x12,0x19,0x9E,0x0D,0x4A,0x00,0x15,0x00,0x15,0x01,0x61,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x14,0x00,0x00,0x20,0x19,0x00,0x00,0x21,0xD5,0x00,0x00,0x00,0x03,0x0C,0x54,0x00,0x39,0x00,0x10,0x00,0x00,0x01,0x01,0x00,0xE1,0x00,0x02,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x31,0x00,0x31,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x12,0x52,0x94,0x01,0xCC,0x01,0xA5,0xD1,0x6F};

      // K8H
      //byte ReadBuffer[] = {0xF7,0x02,0x1E,0x0B,0x3E,0x00,0x39,0x05,0xE1,0x0A,0x68,0x00,0x39,0x05,0xFA,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0xFC,0x00,0x7C,0x00,0x00,0x13,0x87,0x00,0x00,0x24,0x27, 0x02};

      for (uint16_t i = 0; i<sizeof(ReadBuffer); i++) {
        this->DataFrame->push_back(ReadBuffer[i]);
      }
      if (Config->GetDebugLevel() >=4) { dbg.printf("%s\n", this->PrintDataFrame(this->DataFrame).c_str()); }
    #endif
    // ***********************************************
  } 
  
  if (this->DataFrame->size() > 1) {
    tempbyte = this->DataFrame->at(this->DataFrame->size()-1);
    this->DataFrame->pop_back();   // shorten to original size
    
    // setup RequestType
    if (tempbyte == DATAISID) {
      RequestType = "id";
    }
    else if (tempbyte == DATAISLIVE) {
      RequestType = "livedata";
    }

    if (Config->GetDebugLevel() >=3) dbg.printf("parse %d bytes of data\n", this->DataFrame->size());
    if (Config->GetDebugLevel() >=4) dbg.printf("identified datatype: %s\n", RequestType.c_str());
    
    File regfile = LittleFS.open("/regs/"+this->InverterType.filename);
    if (!regfile) {
      dbg.println("failed to open ModbusConfig.json file for writing");
    }
    String streamString = "";
    streamString = "\""+ this->InverterType.name +"\": {";
    regfile.find(streamString.c_str());
    
    streamString = "\""+ RequestType +"\": [";
    regfile.find(streamString.c_str());
    do {
      JsonDocument elem;
      DeserializationError error = deserializeJson(elem, regfile); 
      
      if (!error) {
        // Print the result
        if (Config->GetDebugLevel() >=4) {dbg.println("parsing JSON ok"); }
        if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);}
      } else {
        if (Config->GetDebugLevel() >=1) {
          dbg.print("(Function ParseData) Failed to parse JSON Register Data: "); 
          dbg.print(error.c_str()); 
          dbg.println();
        }
      }

      // setUp local variables
      String datatype = "";
      String openwbtopic = "";
      float factor = 1;
      int valueAdd = 0;
      String unit = "";

      JsonArray posArray, posArray2;
      float val_f = 0;
      int val_i = 0;
      String val_str = "";
      reg_t d = {};
      bool IsActiveItem = false;
      
      
      // mandantory field
      if(!elem["name"].isNull()) {
        d.Name = elem["name"].as<String>();
      } else {
        d.Name = String("undefined");
      }
      
      // mandantory field
      // check if "position" is a well defined array
      if (elem["position"].is<JsonArray>()) {
        posArray = elem["position"].as<JsonArray>();
      } else {
        if (Config->GetDebugLevel() >=1) {
          dbg.printf("Error: for Name '%s' no position array found", d.Name.c_str());
        }
        continue;
      }

      // mandantory field
      if(!elem["datatype"].isNull()) {
        datatype = elem["datatype"].as<String>();
        datatype.toLowerCase();
      } 
      
      // optional field
      if(this->Conf_EnableOpenWBTopic && !elem["openwbtopic"].isNull()) {
        openwbtopic = elem["openwbtopic"].as<String>();
      }
      
      // optional field
      if (elem.containsKey("factor")) {
        factor = elem["factor"];
      }

      // optional field
      if (elem.containsKey("valueAdd")) {
        valueAdd = elem["valueAdd"];
      }
      
      // optional field
      if (elem.containsKey("unit")) {
        unit = elem["unit"].as<String>();
      }
      d.unit = unit;

      // optional field 
      // check, if a extra position for a negative value was defined
      if (elem["position2"].is<JsonArray>()) {
        posArray2 = elem["position2"].as<JsonArray>();
      } 

      // check if item is active to send data out via mqtt
      for (uint16_t i=0; i < this->InverterLiveData->size(); i++) {
        if (this->InverterLiveData->at(i).Name == d.Name && this->InverterLiveData->at(i).active) {
          IsActiveItem = true;
        }
      }
      
      // ************* processing data ******************
      if (datatype == "float") {
        //********** handle Datatype FLOAT ***********//
        val_f = (float)(this->JsonPosArrayToInt(posArray, posArray2) * factor) + valueAdd;
        sprintf(buffer, "%.2f", val_f);
        d.value = String(buffer);
      
      } else if (datatype == "integer") {
        //********** handle Datatype Integer ***********//
        val_i = (this->JsonPosArrayToInt(posArray, posArray2) * factor) + valueAdd;
        sprintf(buffer, "%d", val_i);
        d.value = String(buffer);

      } else if (datatype == "string") {
        //********** handle Datatype String ***********//
        if (!posArray.isNull()){ 
          for(int v : posArray) {
            val_str.concat(String((char)this->DataFrame->at(v)));
          }
        } 
        d.value = val_str;

      } else {
        //****************** sonst, leer *******************//
        d.value = "";
        if (Config->GetDebugLevel() >=2) dbg.printf("Error: for Name '%s\n' no valid datatype found", d.Name.c_str());
      }


      // map values if a mapping is specified
      if(!elem["mapping"].isNull() && elem["mapping"].is<JsonArray>() && d.value != "") {
        if (Config->GetDebugLevel() >=4) dbg.printf("Map values for item %s\n", d.Name.c_str());

        JsonArray map = elem["mapping"].as<JsonArray>();
        d.value = this->MapItem(map, d.value);
      }

      if (Config->GetDebugLevel() >=4) dbg.printf("Data: %s -> %s %s\n", d.Name.c_str(), d.value.c_str(), d.unit.c_str());

      if (this->mqtt && IsActiveItem && d.Name != "") { 
          this->mqtt->Publish_String(d.Name.c_str(), d.value, false);
          if (openwbtopic.length() > 0) { this->mqtt->Publish_String(openwbtopic.c_str(), d.value, true);}
      }

      if(RequestType == "livedata") {
        this->ChangeRegItem(this->InverterLiveData, d);
      } else if(RequestType == "id") {
        this->ChangeRegItem(this->InverterIdData, d);
        
        if (Config->GetDebugLevel() >=3) {
          dbg.printf("Inverter ID Data found -> %s: %s \n", d.Name.c_str(), d.value.c_str());
        }

      }
    
    } while (regfile.findUntil(",","]"));

    if (regfile) { regfile.close(); }

  } 

  // all data were process, clear Dataframe now for next query
  // save this for WebGUI "RAW-Data" first
  if(RequestType == "livedata") {
    this->SaveLiveDataframe->clear();
    this->SaveLiveDataframe->assign(this->DataFrame->begin(), this->DataFrame->end());
  } else if(RequestType == "id") {
    this->SaveIdDataframe->clear();
    this->SaveIdDataframe->assign(this->DataFrame->begin(), this->DataFrame->end());
  }

  this->DataFrame->clear();   
}

/*******************************************************
 * Map a value to a predefined constant string
*******************************************************/
String modbus::MapItem(JsonArray map, String value) {
  String ret = value;

  for (JsonArray mapItem : map) {
    String v1 = mapItem[0].as<String>();
    String v2 = mapItem[1].as<String>();

    
    if (Config->GetDebugLevel() >=5) dbg.printf("Check Map value: %s -> %s\n", v1.c_str(), v2.c_str());

    if (value == v1) {
      ret = v2;
      if (Config->GetDebugLevel() >=4) dbg.printf("Mapped value: %s -> %s", v1.c_str(), v2.c_str());
    }
  } 
  return ret;
}

/*******************************************************
 * Change a Register Item with current data
*******************************************************/
void modbus::ChangeRegItem(std::vector<reg_t>* vector, reg_t item) {
  for (uint16_t i=0; i<vector->size(); i++) {
    if (vector->at(i).Name == item.Name) {
      vector->at(i).value = item.value;
      vector->at(i).unit = item.unit;
      break;
    }
  }
}

/*******************************************************
 * Calculate CRC Checksum
*******************************************************/
uint16_t modbus::Calc_CRC(uint8_t* message, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)message[pos];          // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--) {          // Loop over each bit
      if ((crc & 0x0001) != 0) {            // If the LSB is set
        crc >>= 1;                          // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                                  // Else LSB is not set
        crc >>= 1;                          // Just shift right
    }
  }
  return crc;
}  

uint16_t modbus::Calc_CRC(std::vector<byte>* message, uint16_t startpos, uint16_t endpos) {
  uint16_t crc = 0xFFFF;
//  dbg.print("Calc_CRC of: ");
  for (uint16_t pos = startpos; pos < endpos; pos++) {
    crc ^= (uint16_t)message->at(pos);      // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--) {          // Loop over each bit
      if ((crc & 0x0001) != 0) {            // If the LSB is set
        crc >>= 1;                          // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                                  // Else LSB is not set
        crc >>= 1;                          // Just shift right
    }
//  dbg.printf("%s ",this->PrintHex(message->at(pos)).c_str());
  }
//  dbg.println();
//  dbg.printf("Calculated CRC (%d values): 0x%02X 0x%02X\n", endpos-startpos, highByte(crc), lowByte(crc));
  return crc;
}

/*******************************************************
 * friendly output of hex nums
*******************************************************/
String modbus::PrintHex(byte num) {
  char hexCar[5] = {0};
  memset(hexCar, 0, sizeof(hexCar));
  snprintf(hexCar, sizeof(hexCar), "0x%02X", num);
  return String(hexCar);
}

/*******************************************************
 * friendly output the entire received Dataframe
*******************************************************/
String modbus::PrintDataFrame(std::vector<byte>* frame) {
  String out = "";
  for (uint16_t i = 0; i<frame->size(); i++) {
    if (out.length() < 2040) { // max string length 2048
      out.concat(this->PrintHex(frame->at(i)));
      out.concat(" ");
    }
  }
  return out;
}

String modbus::PrintDataFrame(byte* frame, uint8_t len) {
  String out = "";
  for (uint16_t i = 0; i<len; i++) {
    out.concat(this->PrintHex(frame[i]));
    out.concat(" ");
  }
  return out;
}

/*******************************************************
 * make InverterSN callable from outside
*******************************************************/
String modbus::GetInverterSN() {
  String sn = "unknown";
  
  for (uint16_t i=0; i < this->InverterIdData->size(); i++) {
    if (this->InverterIdData->at(i).Name == "InverterSN")
      return this->InverterIdData->at(i).value;
  }
  return sn;
}

/*******************************************************
 * Return all LiveData as jsonArray
 * {data: [{"name": "xx", "value": "xx", ...}, ...] }
*******************************************************/
void modbus::GetLiveDataAsJson(AsyncResponseStream *response, String subaction) {
  int count = 0;
  response->print("{\"data\": {\"items\": ["); 
  
  for (uint16_t i=0; i < this->InverterLiveData->size(); i++) {
    if (subaction == "onlyactive" && !this->InverterLiveData->at(i).active)  continue;
    JsonDocument doc;
    String s = "";
    doc["name"]  = this->InverterLiveData->at(i).Name;
    doc["realname"]  = this->InverterLiveData->at(i).RealName;
    doc["value"] = this->InverterLiveData->at(i).value + " " + this->InverterLiveData->at(i).unit;
    doc["active"].to<JsonObject>();
    doc["active"]["checked"] = (this->InverterLiveData->at(i).active?1:0);
    doc["active"]["name"] = this->InverterLiveData->at(i).Name;
    doc["mqtttopic"] = this->mqtt->getTopic(this->InverterLiveData->at(i).Name, false);

    if (this->InverterLiveData->at(i).openwb.length() > 0) {
      JsonArray wb = doc["openwb"].to<JsonArray>();
      wb[0]["openwbtopic"]  = this->InverterLiveData->at(i).openwb;
    }

    serializeJson(doc, s);
    if(count>0) response->print(", ");
    response->print(s);
    count++;
  }
  
  response->printf(" ]}, \"object_id\": \"%s/%s\"}", Config->GetMqttBasePath().c_str(), Config->GetMqttRoot().c_str());
}

/*******************************************************
 * Return all LiveData as jsonArray
 * {data: [{"name": "xx", "value": "xx"}], }
 * {"GridVoltage_R":"0.00 V","GridCurrent_R":"0.00 A","GridPower_R":"0 W","GridFrequency_R":"0.00 Hz","GridVoltage_S":"0.90 V","GridCurrent_S":"1715.40 A","GridPower_S":"-28671 W","GridFrequency_S":"174.08 Hz","GridVoltage_T":"0.00 V","GridCurrent_T":"0.00 A","GridPower_T":"0 W","GridFrequency_T":"1.30 Hz","PvVoltage1":"259.80 V","PvVoltage2":"0.00 V","PvCurrent1":"1.00 A","PvCurrent2":"0.00 A","Temperature":"28 &deg;C","PowerPv1":"283 W","PowerPv2":"0 W","BatVoltage":"0.00 V","BatCurrent":"0.00 A","BatPower":"0 W","BatTemp":"0 &deg;C","BatCapacity":"0 %","OutputEnergyChargeWh":"0 Wh","OutputEnergyChargeKWh":"0.00 KWh","OutputEnergyChargeToday":"0.00 KWh","InputEnergyChargeWh":"0 Wh","InputEnergyChargeKWh":"0.00 KWh"}
*******************************************************/
void modbus::GetRegisterAsJson(AsyncResponseStream *response) {
  int count = 0;
  response->print("{\"data\": ["); 
  
  File regfile = LittleFS.open("/regs/"+this->InverterType.filename);
  if (!regfile) {
    dbg.println("failed to open ModbusConfig.json file for writing");
    return;
  }
  String streamString = "";
  streamString = "\""+ this->InverterType.name +"\": {";
  regfile.find(streamString.c_str());

  streamString = "\"livedata\": [";
  regfile.find(streamString.c_str());

  do {
    JsonDocument elem;
    DeserializationError error = deserializeJson(elem, regfile); 
      
    if (!error) {
      // Print the result
      if (Config->GetDebugLevel() >=4) {dbg.println("parsing JSON ok"); }
      if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);}
    } else {
      if (Config->GetDebugLevel() >=1) {
        dbg.print("(Function GetRegisterAsJson) Failed to parse JSON Register Data: "); 
        dbg.print(error.c_str()); 
        dbg.println();
      }
    }
  
    String s = "";
    serializeJson(elem, s);
    if(count>0) response->print(", ");
    response->print(s);
    count++;

  } while (regfile.findUntil(",","]"));

  if (regfile) { regfile.close(); }
  response->print("]}");
}

/*******************************************************
 * request for changing active Status of a certain item
 * Used by handleAjax function
*******************************************************/
void modbus::SetItemActiveStatus(String item, bool newstate) {
  for (uint16_t j=0; j < this->InverterLiveData->size(); j++) {
    if (this->InverterLiveData->at(j).Name == item) {
      if (Config->GetDebugLevel() >=3) {
        dbg.printf("Set Item <%s> ActiveState to %s\n", item.c_str(), (newstate?"true":"false"));
      }
      this->InverterLiveData->at(j).active = newstate;
    }
  }
}

/*******************************************************
 * loop function
*******************************************************/
void modbus::loop() {
  if (millis() - this->LastTxLiveData > this->TxIntervalLiveData * 1000) {
    this->LastTxLiveData = millis();
    
    if (this->InverterType.filename.length() > 1) {
      this->QueryLiveData();
      if (this->enableRelays) this->ReadRelays();
    }  
   }

  if (millis() - this->LastTxIdData > this->TxIntervalIdData * 1000) {
    this->LastTxIdData = millis();
    
    if (this->InverterType.filename.length() > 1) {this->QueryIdData();}
  }

  //its allowed to send a new request every 800ms, we use recommend 1000ms
  if (millis() - this->LastTxInverter > 1000) {
    this->LastTxInverter = millis();

    if (this->InverterType.filename.length() > 1) {this->QueryQueueToInverter();}
  }
}

/*******************************************************
 * load initial Register Items from file into vector
*******************************************************/
void modbus::LoadRegItems(std::vector<reg_t>* vector, String type) {
  vector->clear();

  if (Config->GetDebugLevel() >=4) {
    dbg.printf("Load RegItems for Inverter %s and type <%s>\n", this->InverterType.name.c_str(), type.c_str());
  }

  File regfile = LittleFS.open("/regs/"+this->InverterType.filename);
  if (!regfile) {
    dbg.println("failed to open ModbusConfig.json file for writing");
    return;
  }

  String streamString = "";
  streamString = "\""+ this->InverterType.name +"\": {";
  regfile.find(streamString.c_str());
    
  streamString = "\"" + type + "\": [";
  regfile.find(streamString.c_str());
  do {
    JsonDocument elem;
    DeserializationError error = deserializeJson(elem, regfile); 
      
    if (!error) {
      // Print the result
      if (Config->GetDebugLevel() >=4) {dbg.println("parsing JSON ok"); }
      if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);}
    } else {
      if (Config->GetDebugLevel() >=1) {
        dbg.printf("(Function LoadRegItems) Failed to parse JSON Register Data for Inverter <%s> and type <%s>: %s\n", this->InverterType.name.c_str(), type.c_str(), error.c_str());
      }
    }

    reg_t d = {};
      
    // mandantory field
    if(!elem["name"].isNull()) {
      d.Name = elem["name"].as<String>();
    } else {
      d.Name = String("undefined");
    }

    // optional field
    if(!elem["realname"].isNull()) {
      d.RealName = elem["realname"].as<String>();
    } else {
      d.RealName = d.Name;
    }

    // optional field
    if(!elem["openwbtopic"].isNull()) {
      d.openwb = elem["openwbtopic"].as<String>();
    } 

    d.active = false; // set initial
    vector->push_back(d);

    if (Config->GetDebugLevel() >=4) {
      dbg.printf("processed RegItem: %s\n", d.Name.c_str());
    }

  } while (regfile.findUntil(",","]"));

  if (regfile) { regfile.close(); }
}

/*******************************************************
 * load configuration from file
*******************************************************/
void modbus::LoadJsonConfig(bool firstrun) {
  bool loadDefaultConfig = false;
  uint32_t Baudrate_old = this->Baudrate;
  uint8_t pin_RX_old    = this->pin_RX;
  uint8_t pin_TX_old    = this->pin_TX;
  uint8_t pin_RTS_old   = this->pin_RTS;
  regfiles_t InverterType_old = this->InverterType;
  uint8_t pin_Relay1_old   = this->pin_Relay1;
  uint8_t pin_Relay2_old   = this->pin_Relay2;
  bool enableRelays_old  = this->enableRelays;

  if (LittleFS.exists("/modbusconfig.json")) {
    //file exists, reading and loading
    if (Config->GetDebugLevel() >=3) dbg.println("reading config file....");
    File configFile = LittleFS.open("/modbusconfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) dbg.println("config file is open:");
      //size_t size = configFile.size();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error && doc.containsKey("data")) {
        if (Config->GetDebugLevel() >=3) { serializeJsonPretty(doc, dbg); dbg.println(); }
        
        if (doc["data"].containsKey("pin_rx"))           { this->pin_RX = (int)(doc["data"]["pin_rx"]);} else {this->pin_RX = this->default_pin_RX;}
        if (doc["data"].containsKey("pin_tx"))           { this->pin_TX = (int)(doc["data"]["pin_tx"]);} else {this->pin_TX = this->default_pin_TX;}
        if (doc["data"].containsKey("pin_rts"))          { this->pin_RTS = (int)(doc["data"]["pin_rts"]);} else {this->pin_RTS = this->default_pin_RTS;}
        if (doc["data"].containsKey("clientid"))         { this->ClientID = strtoul(doc["data"]["clientid"], NULL, 16);} else {this->ClientID = 0x01;} // hex convert to dec
        if (doc["data"].containsKey("baudrate"))         { this->Baudrate = (int)(doc["data"]["baudrate"]);} else {this->Baudrate = 19200;}
        if (doc["data"].containsKey("txintervallive"))   { this->TxIntervalLiveData = (int)(doc["data"]["txintervallive"]);} else {this->TxIntervalLiveData = 5;}
        if (doc["data"].containsKey("txintervalid"))     { this->TxIntervalIdData = (int)(doc["data"]["txintervalid"]);} else {this->TxIntervalIdData = 3600;}
        if (doc["data"].containsKey("enable_openwbtopic")){ this->Conf_EnableOpenWBTopic = (doc["data"]["enable_openwbtopic"]).as<bool>();} else { this->Conf_EnableOpenWBTopic = false; }
        if (doc["data"].containsKey("enable_setters"))   { this->Conf_EnableSetters = (doc["data"]["enable_setters"]).as<bool>();} else { this->Conf_EnableSetters = false; }
        if (doc["data"].containsKey("enableCrcCheck"))   { this->enableCrcCheck = (doc["data"]["enableCrcCheck"]).as<bool>();} else { this->enableCrcCheck = true; }
        if (doc["data"].containsKey("enableLengthCheck")){ this->enableLengthCheck = (doc["data"]["enableLengthCheck"]).as<bool>();} else { this->enableLengthCheck = true; }
        if (doc["data"].containsKey("pin_RELAY1"))       { this->pin_Relay1= (int)(doc["data"]["pin_RELAY1"]);} else {this->pin_Relay1 = this->default_pin_Relay1;}
        if (doc["data"].containsKey("pin_RELAY2"))       { this->pin_Relay2 = (int)(doc["data"]["pin_RELAY2"]);} else {this->pin_Relay2 = this->default_pin_Relay2;}
        if (doc["data"].containsKey("EnableRelays"))     { if (doc["data"]["EnableRelays"].as<String>()=="1") this->enableRelays = true; else this->enableRelays = false;} else { this->enableRelays = false; }
        

        if (doc["data"].containsKey("invertertype"))     { 
          bool found = false;
          for (uint8_t i=0; i<this->AvailableInverters->size(); i++) {
            if (this->AvailableInverters->at(i).name == (doc["data"]["invertertype"]).as<String>()) {
              this->InverterType = this->AvailableInverters->at(i); 
              if (Config->GetDebugLevel() >=3) {
                dbg.printf("Invertertyp '%s' was found in register file '%s', set it as selected active Inverter\n", this->InverterType.name.c_str(), this->InverterType.filename.c_str());
              }
              found = true;
            } 
          }
          if (!found) { 
            if (this->AvailableInverters->size()>0) {
              this->InverterType = this->AvailableInverters->at(0);
            }
            if (Config->GetDebugLevel() >=3) {
                dbg.printf("Invertertyp '%s' was not found, use default '%s' instead\n", (doc["data"]["invertertype"]).as<String>().c_str(), this->InverterType.name.c_str());
            }
          }
        }

      } else {
        if (Config->GetDebugLevel() >=1) {dbg.println("failed to load modbus json config, load default config");}
        loadDefaultConfig = true;
      }
      configFile.close();
    }
  } else {
    if (Config->GetDebugLevel() >=3) {dbg.println("modbusconfig.json config File not exists, load default config");}
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    if (this->AvailableInverters->size()>0) {
      this->InverterType = this->AvailableInverters->at(0);
    } 

    this->pin_RX = this->default_pin_RX;
    this->pin_TX = this->default_pin_TX;
    this->pin_RTS = this->default_pin_RTS;
    this->ClientID = 0x01;
    this->Baudrate = 19200;
    this->TxIntervalLiveData = 5;
    this->TxIntervalIdData = 3600;
    this->pin_Relay1 = this->default_pin_Relay1;
    this->pin_Relay2 = this->default_pin_Relay2;
    this->enableRelays = false;
    this->Conf_EnableOpenWBTopic = false;
    this->Conf_EnableSetters = false;

    loadDefaultConfig = false; //set back
  }

  // ReInit if Baudrate was changed, not at firstrun!
  if(!firstrun && (
     (Baudrate_old != this->Baudrate) ||
     (pin_RX_old   != this->pin_RX)   ||
     (pin_TX_old   != this->pin_TX)   ||
     (pin_RTS_old  != this->pin_RTS)  ||
     (enableRelays_old != this->enableRelays) ||
     (pin_Relay1_old  != this->pin_Relay1)  ||
     (pin_Relay2_old  != this->pin_Relay2))) { 
    
    this->init(false);
  }

  // ReInit if Invertertype was changed
  if(!firstrun && (
    InverterType_old.name != this->InverterType.name) ) { 
    
    this->init(false);
  }

}

/*******************************************************
 * load Modbus Item configuration from file
*******************************************************/
void modbus::LoadJsonItemConfig() {
  
  if (LittleFS.exists("/modbusitemconfig.json")) {
    //file exists, reading and loading
    if (Config->GetDebugLevel() >=3) dbg.println("reading modbus item config file....");
    File configFile = LittleFS.open("/modbusitemconfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) dbg.println("modbus item config file is open:");

      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {
        JsonDocument elem;
        DeserializationError error = deserializeJson(elem, stream); 

        if (!error) {
          // Print the result
          if (Config->GetDebugLevel() >=4) {dbg.println("parsing JSON ok"); }
          if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, dbg);}
        } else {
          if (Config->GetDebugLevel() >=1) {
            dbg.print("(Function LoadJsonItemConfig) Failed to parse JSON Register Data: "); 
            dbg.print(error.c_str()); 
            dbg.println();
          }
        }

        for (JsonPair kv : elem.as<JsonObject>()) {
          const char* ItemName = kv.key().c_str();

          //dbg.println(kv.key().c_str());
          //dbg.println(kv.value().as<const char*>());

          for(uint16_t i=0; i<this->InverterLiveData->size(); i++) {
            if (this->InverterLiveData->at(i).Name == ItemName ) {
              this->InverterLiveData->at(i).active = kv.value().as<bool>();

              if (Config->GetDebugLevel() >=3) {
                dbg.printf("item %s -> %s\n", ItemName, (this->InverterLiveData->at(i).active?"enabled":"disabled"));
              }

              break;
            }
          }
        }

      } while (stream.findUntil(",","]"));
      configFile.close();
    } else {
      if (Config->GetDebugLevel() >=1) {dbg.println("failed to load modbusitemconfig.json, load default item config");}
    }
  } else {
    if (Config->GetDebugLevel() >=3) {dbg.println("modbusitemconfig.json config File not exists, all items are inactive as default");}
  }
}

/*******************************************************************************************************
 * WebContent
*******************************************************************************************************/
void modbus::GetInitData(AsyncResponseStream *response) {
  String ret;
  JsonDocument json;
  json["data"].to<JsonObject>();
  json["data"]["GpioPin_RX"]          = this->pin_RX;
  json["data"]["GpioPin_TX"]          = this->pin_TX;
  json["data"]["GpioPin_RTS"]         = this->pin_RTS;
  json["data"]["clientid"]            = this->ClientID;
  json["data"]["baudrate"]            = this->Baudrate;
  json["data"]["txintervallive"]      = this->TxIntervalLiveData;
  json["data"]["txintervalid"]        = this->TxIntervalIdData;
  json["data"]["GpioPin_Relay1"]      = this->pin_Relay1;
  json["data"]["GpioPin_Relay2"]      = this->pin_Relay2;
  json["data"]["EnableRelays_On"]     = ((this->enableRelays)?1:0);
  json["data"]["EnableRelays_Off"]    = ((this->enableRelays)?0:1);

  json["data"]["enable_openwbtopic"]  = ((this->Conf_EnableOpenWBTopic)?1:0);
  json["data"]["enableCrcCheck"]      = ((this->enableCrcCheck)?1:0);
  json["data"]["enableLengthCheck"]      = ((this->enableLengthCheck)?1:0);
  json["data"]["enable_setters"]      = ((this->Conf_EnableSetters)?1:0);
  
  for (uint8_t i=0; i< AvailableInverters->size(); i++) {
    json["data"]["inverters"][i]["inverter"].to<JsonObject>();
    json["data"]["inverters"][i]["inverter"]["value"] = AvailableInverters->at(i).name;
    json["data"]["inverters"][i]["inverter"]["selected"] = (AvailableInverters->at(i).name == this->InverterType.name?1:0);
    json["data"]["inverters"][i]["inverter"]["text"] = AvailableInverters->at(i).name;
  }

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);
  response->print(ret);
}

void modbus::GetInitRawData(AsyncResponseStream *response) {
  String ret = "";
  std::ostringstream id, live;
  JsonDocument json;

  live << std::hex << std::uppercase;
  id << std::hex << std::uppercase;

  for (uint16_t i = 0; i < this->SaveIdDataframe->size(); i++) {
    id << std::setw(2) << std::setfill('0') << (int)this->SaveIdDataframe->at(i);
  }

  for (uint16_t i = 0; i < this->SaveLiveDataframe->size(); i++) {
    live << std::setw(2) << std::setfill('0') << (int)this->SaveLiveDataframe->at(i);
  }

  json["data"].to<JsonObject>();
  json["data"]["id_rawdata_org"] = id.str();
  json["data"]["live_rawdata_org"] = live.str();

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);

  response->print(ret);
}