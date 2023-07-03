#include "modbus.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : Baudrate(19200), LastTxLiveData(0), LastTxIdData(0), LastTxInverter(0), pin_RX(16), pin_TX(17), pin_RTS(18) {
  DataFrame           = new std::vector<byte>{};
  SaveIdDataframe     = new std::vector<byte>{};
  SaveLiveDataframe   = new std::vector<byte>{};

  InverterLiveData    = new std::vector<reg_t>{};
  InverterIdData      = new std::vector<reg_t>{};
  AvailableInverters  = new std::vector<String>{};
  Setters             = new std::vector<subscription_t>{};

  Conf_RequestLiveData= new std::vector<std::vector<byte>>{};
  Conf_RequestIdData  = new std::vector<byte>{};

  ReadQueue = new ArduinoQueue<std::vector<byte>>(5); // max 5 read requests parallel
  SetQueue  = new ArduinoQueue<std::vector<byte>>(5); // max 5 set requests parallel

  this->LoadJsonConfig(true);

  this->init(true);
}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init(bool firstrun) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  if (Config->GetDebugLevel() >=3) {
    sprintf(dbg, "Start Hardwareserial 1 on RX (%d), TX(%d), RTS(%d)", this->pin_RX, this->pin_TX, this->pin_RTS);
    Serial.println(dbg);
    sprintf(dbg, "Init Modbus to Client 0x%02X with %d Baud", this->ClientID, this->Baudrate);
    Serial.println(dbg);
  }

  // Configure Direction Control pin
  pinMode(this->pin_RTS, OUTPUT);  

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
 * generate the full mqtt topic without /# at end
*******************************************************/
String modbus::GetMqttSetTopic(String command) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  snprintf(dbg, sizeof(dbg), "%s/%s/set/%s", Config->GetMqttBasePath().c_str(), Config->GetMqttRoot().c_str(), command.c_str());
  return (String)dbg;
}

/*******************************************************
 * subscribe to all possible "set" register (register.h)
*******************************************************/
void modbus::GenerateMqttSubscriptions() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  // clear vector
  this->Setters->clear();
  ProgmemStream stream{JSON};
  String streamString = "";
  streamString = "\""+ this->InverterType +"\": {";
  stream.find(streamString.c_str());
  streamString = "\"set\": [";
  stream.find(streamString.c_str());
  do {
    StaticJsonDocument<512> elem;
    DeserializationError error = deserializeJson(elem, stream); 
    if (!error) {
      // Print the result
      if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON for <set> data ok"); }
      if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);}
     
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
          snprintf(dbg, sizeof(dbg), "Set command successfully parsed from JSON: %s with %s", s.command, (this->PrintDataFrame(&(s.request))).c_str());
          Serial.println(dbg);
        }
        this->Setters->push_back(s);

      } else {
        continue;
      }
     
    } else {
      if (Config->GetDebugLevel() >=1) {
        Serial.print("Failed to parse JSON Register <set> Data: "); 
        Serial.print(error.c_str()); 
        Serial.println();
      }
    }
  } while (stream.findUntil(",","]"));

}


/*******************************************************
 * act on received mqtt command
*******************************************************/
void modbus::ReceiveMQTT(String topic, int msg) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  if (!this->Conf_EnableSetters) {
    if (Config->GetDebugLevel() >=2) {
      snprintf(dbg, sizeof(dbg), "Set command <%s> received, but setters over mqtt are currently disabled", topic.c_str());
      Serial.println(dbg);
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
        snprintf(dbg, sizeof(dbg), "MQTT Setter found: %s" ,this->Setters->at(i).command.c_str());
        Serial.println(dbg);
        snprintf(dbg, sizeof(dbg), "Initiate Set Request to queue: %s" ,(this->PrintDataFrame(&request)).c_str());
        Serial.println(dbg);
      }

      this->SetQueue->enqueue(request);
    }
  }
}

/*******************************************************
 * get all defined inverters from json (register.h)
*******************************************************/
void modbus::LoadInvertersFromJson() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  StaticJsonDocument<512> regjson;
  StaticJsonDocument<128> filter;

  AvailableInverters->clear();

  filter["*"]["config"]["ClientIdPos"] = true;  
  
  DeserializationError error = deserializeJson(regjson, JSON, DeserializationOption::Filter(filter));

  if (!error) {
    // https://arduinojson.org/v6/api/jsonobject/begin_end/
    JsonObject root = regjson.as<JsonObject>();
    for (JsonPair kv : root) {
      if (Config->GetDebugLevel() >=3) {
        sprintf(dbg, "Inverter found: %s", kv.key().c_str());
        Serial.println(dbg);
      }

      AvailableInverters->push_back(kv.key().c_str());
    }
  } else{
    sprintf(dbg, "Error: unable to load inverters form JSON: %s", error.c_str());
  }
}

/*******************************************************
 * read config data from JSON, part "config"
*******************************************************/
void modbus::LoadInverterConfigFromJson() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  StaticJsonDocument<1224> doc;
  StaticJsonDocument<100> filter;

  filter[this->InverterType]["config"] = true;
    
  DeserializationError error = deserializeJson(doc, JSON, DeserializationOption::Filter(filter));

  if (error && Config->GetDebugLevel() >=1) {
    sprintf(dbg, "Error: unable to read configdata for inverter %s: %s", this->InverterType, error.c_str());
    Serial.println(dbg);
  } else {
    if (Config->GetDebugLevel() >=4) {
      sprintf(dbg, "Read config data for inverter %s", this->InverterType.c_str());
      Serial.println(dbg);
      serializeJsonPretty(doc, Serial);
      Serial.println();
    }
  }

  this->Conf_LiveDataFunctionCode   = this->String2Byte(doc[this->InverterType]["config"]["LiveDataFunctionCode"].as<String>());
  this->Conf_IdDataFunctionCode     = this->String2Byte(doc[this->InverterType]["config"]["IdDataFunctionCode"].as<String>());
  this->Conf_LiveDataErrorCode      = this->String2Byte(doc[this->InverterType]["config"]["LiveDataErrorCode"].as<String>());
	this->Conf_IdDataErrorCode        = this->String2Byte(doc[this->InverterType]["config"]["IdDataErrorCode"].as<String>());
	this->Conf_LiveDataSuccessCode    = this->String2Byte(doc[this->InverterType]["config"]["LiveDataSuccessCode"].as<String>());
	this->Conf_IdDataSuccessCode      = this->String2Byte(doc[this->InverterType]["config"]["IdDataSuccessCode"].as<String>());
  this->Conf_ClientIdPos            = int(doc[this->InverterType]["config"]["ClientIdPos"]);
  this->Conf_LiveDataStartsAtPos    = int(doc[this->InverterType]["config"]["LiveDataStartsAtPos"]);
	this->Conf_IdDataStartsAtPos      = int(doc[this->InverterType]["config"]["IdDataStartsAtPos"]);
	this->Conf_LiveDataErrorPos       = int(doc[this->InverterType]["config"]["LiveDataErrorPos"]);
	this->Conf_IdDataErrorPos         = int(doc[this->InverterType]["config"]["IdDataErrorPos"]);
	this->Conf_LiveDataSuccessPos     = int(doc[this->InverterType]["config"]["LiveDataSuccessPos"]);
	this->Conf_IdDataSuccessPos       = int(doc[this->InverterType]["config"]["IdDataSuccessPos"]);
  this->Conf_IdDataFunctionCodePos  = int(doc[this->InverterType]["config"]["IdDataFunctionCodePos"]);
  this->Conf_LiveDataFunctionCodePos= int(doc[this->InverterType]["config"]["LiveDataFunctionCodePos"]);
  
  Conf_RequestLiveData->clear();
  for (JsonArray arr : doc[this->InverterType]["config"]["RequestLiveData"].as<JsonArray>()) {
  
    std::vector<byte> t = {};
    for (String x : arr) {
      byte e = this->String2Byte(x);
      t.push_back(e);
    }
    Serial.println();
    Conf_RequestLiveData->push_back(t);
  }
  
  Conf_RequestIdData->clear();
  for (String elem : doc[this->InverterType]["config"]["RequestIdData"].as<JsonArray>()) {
    byte e = this->String2Byte(elem);
    Conf_RequestIdData->push_back(e);
  }

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
  Serial.println("MQTT aktiviert");

  this->GenerateMqttSubscriptions();
}

/*******************************************************
 * Query ID Data to Inverter
*******************************************************/
void modbus::QueryIdData() {
  if (Config->GetDebugLevel() >=4) {Serial.println("Query ID Data into Queue:");}
  
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
    if (Config->GetDebugLevel() >=4) { Serial.println(this->PrintDataFrame(this->Conf_RequestIdData).c_str()); }
    this->ReadQueue->enqueue(*this->Conf_RequestIdData);
  }
}


/*******************************************************
 * Query Live Data to Inverter
*******************************************************/
void modbus::QueryLiveData() {
  if (Config->GetDebugLevel() >=4) {Serial.println("Query Live Data into Queue:"); }
   
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
      if (Config->GetDebugLevel() >=4) { Serial.println(this->PrintDataFrame(&this->Conf_RequestLiveData->at(i)).c_str()); }
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

  if (!this->SetQueue->isEmpty()) {
    rwtype = WRITE;
    m = this->SetQueue->dequeue();
  } // writing has priority
  else if (!this->ReadQueue->isEmpty()) {
    rwtype = READ;
    m = this->ReadQueue->dequeue();
  }
  else { rwtype = NUL; }

  if (rwtype != NUL) {
    if (Config->GetDebugLevel() >=3) { Serial.print("Request queue data to inverter: "); }
  
    digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
    while (RS485Serial->available() > 0) { // read serial if any old data is available
      RS485Serial->read();
    }

    byte message[m.size() + 2] = {0x00}; // +2 Byte CRC
  
    for (uint8_t i; i < m.size(); i++) {
      message[i] = m.at(i);
    }

    uint16_t crc = this->Calc_CRC(message, sizeof(message)-2);
    message[sizeof(message)-1] = highByte(crc);
    message[sizeof(message)-2] = lowByte(crc);
    m.push_back(lowByte(crc));
    m.push_back(highByte(crc));

    if (Config->GetDebugLevel() >=3) { Serial.println(this->PrintDataFrame(message, sizeof(message))); }

    digitalWrite(this->pin_RTS, RS485Transmit);     // init Transmit
    RS485Serial->write(message, sizeof(message));
    RS485Serial->flush();
    digitalWrite(this->pin_RTS, RS485Receive); 

    unsigned long timeout=millis()+100;
    while (millis()<=timeout) { yield(); }

    if (rwtype == WRITE) {
      this->ReceiveSetData(&m);
    } 
    else if (rwtype == READ) { 
      this->ReceiveReadData();
      if (this->ReadQueue->isEmpty()) {
        this->ParseData();
      }
    }

  }
}

/***********************************************************
 * Receive Data after a Set Query, Check if successful set
************************************************************/
bool modbus::ReceiveSetData(std::vector<byte>* SendHexFrame) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  std::vector<byte> RecvHexframe = {};
  bool ret = false; 

  if (Config->GetDebugLevel() >=3) {Serial.println("Read Data from Queue: ");}

  digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
  if (RS485Serial->available()) {
    // lese alle Daten, speichern im Vektor "Dataframe"
    while(RS485Serial->available()) {
      byte d = RS485Serial->read();
      RecvHexframe.push_back(d);
      if (Config->GetDebugLevel() >=4) {Serial.print(PrintHex(d)); Serial.print(" ");}
      delay(1); // keep this! Loosing bytes possible if too fast
    }    
    if (Config->GetDebugLevel() >=4) {Serial.println();}
    
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
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  if (Config->GetDebugLevel() >=3) {Serial.println("Read Data from Queue: ");}

  digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
  if (RS485Serial->available()) {
    // lese alle Daten, speichern im Vektor "Dataframe"
    while(RS485Serial->available()) {
      byte d = RS485Serial->read();
      this->DataFrame->push_back(d);
      if (Config->GetDebugLevel() >=4) {Serial.print(PrintHex(d)); Serial.print(" ");}
      delay(1); // keep this! Loosing bytes possible if too fast
    }    
    if (Config->GetDebugLevel() >=4) {Serial.println();}
    
    if (this->DataFrame->size() > 5 &&
        this->DataFrame->at(this->Conf_ClientIdPos) == this->ClientID && 
        this->DataFrame->at(this->Conf_IdDataErrorPos) != this->Conf_IdDataErrorCode && 
        this->DataFrame->at(this->Conf_LiveDataErrorPos) != this->Conf_LiveDataErrorCode) {
      // Dataframe valid
      if (Config->GetDebugLevel() >=3) {
        sprintf(dbg, "Dataframe valid, Dateframe size: %d bytes", this->DataFrame->size());
        Serial.println(dbg);
      }
    } else {
      if (Config->GetDebugLevel() >=2) {Serial.println("Dataframe invalid");}
    }
  } else {
    if (Config->GetDebugLevel() >=2) {Serial.println("no response from client");}
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
  String RequestType = "";
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  if (Config->GetDebugLevel() >=3) {
    sprintf(dbg, "Parse %d Bytes of data", this->DataFrame->size());
    Serial.println(dbg);
  }

  if (this->DataFrame->size() == 0) {

    // TODO
    //  ***********************************************
    // do some tests if client isn´t connected, dataframe is empty
    //  ***********************************************
    #ifdef DEBUGMODE 
      if (Config->GetDebugLevel() >=3) {Serial.println("Start parsing in testmode, use some testdata instead real live data :)");}
      
      // Solar-KTL 
      //byte ReadBuffer[] = {0x01, 0x03, 0x60, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x34, 0x01, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x13, 0x86, 0x09, 0x11, 0x01, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4C, 0x00, 0x00, 0x02, 0x5F, 0x00, 0x8A, 0x01, 0x7D, 0x00, 0x28, 0x00, 0x33, 0x0E, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x60, 0x01, 0x42, 0x0A, 0x3B, 0x00, 0x0E, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x25};

      // Solax MIC
      //byte ReadBuffer[] = {0x01, 0x04, 0x80, 0x12, 0x34, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x09, 0x64, 0x09, 0x67, 0x09, 0x6B, 0x13, 0x8C, 0x13, 0x8D, 0x13, 0x8B, 0x00, 0x27, 0x00, 0x28, 0x00, 0x28, 0x00, 0x2C, 0x0B, 0x54, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x70, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5B, 0xC3};
      
      //Solax X1
      byte ReadBuffer[] = {0x01, 0x04, 0xA6, 0x08, 0xF4, 0x00, 0x0D, 0x01, 0x0D, 0x0A, 0x26, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x13, 0x8B, 0x00, 0x1C, 0x00, 0x02, 0x01, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE6, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x09, 0x17, 0x04, 0x56};
      //byte ReadBuffer[] = {0x01, 0x03, 0x28, 0x48, 0x34, 0x35, 0x30, 0x32, 0x41, 0x49, 0x34, 0x34, 0x35, 0x39, 0x30, 0x30, 0x35, 0x73, 0x6F, 0x6C, 0x61, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4A, 0xA0};
      
      //Growatt
      //byte ReadBuffer[] = {0x01,0x04,0xEE,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x79,0x01,0x13,0x88,0x10,0x02,0x00,0x2A,0x00,0x00,0x79,0x7D,0x10,0x08,0x00,0x2B,0x00,0x00,0x00,0x00,0x0F,0xCE,0x00,0x2A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x5B,0x00,0x00,0x05,0xE6,0x00,0x1D,0x2C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x54,0x01,0x47,0x01,0x69,0x00,0x00,0x0A,0x02,0x1C,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x83,0x00,0x00,0x02,0xE8,0x00,0x00,0x00,0x00,0x00,0x00,0x7A,0x44};
      
      for (uint8_t i = 0; i<sizeof(ReadBuffer); i++) {
        this->DataFrame->push_back(ReadBuffer[i]);
        if (Config->GetDebugLevel() >=4) {Serial.print(PrintHex(ReadBuffer[i])); Serial.print(" ");}
      }
      
      //byte ReadBuffer2[] = {0x01,0x04,0xEE,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x78,0x00,0x00,0x00,0x00,0x0A,0x05,0x00,0x1B,0x00,0x00,0x00,0x00,0x05,0x3A,0xFE,0x79,0xFC,0x49,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x43,0x02,0x90,0x00,0x44,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x78,0x00,0x00,0x04,0x09,0x04,0x17,0x03,0xFB,0x00,0x00,0x82,0x78,0x00,0x00,0x00,0xEF,0x00,0x06,0x1C,0x1C,0x00,0xFA,0x00,0x00,0x00,0x26,0x00,0x00,0x06,0xC1,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x57,0x00,0x00,0x01,0x5D,0x00,0x00,0x00,0x6E,0x00,0x00,0x01,0xB9,0x00,0x00,0x00,0x6F,0x00,0x00,0x07,0x5F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x2F,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x1A,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x00,0x00,0x00,0x00,0x00,0x1B,0x0A,0x05,0xFB,0x00,0x01,0x20,0x00,0xFA,0x04,0xE2,0x11,0xBC,0x00,0x00,0x00,0x18,0x00,0x03,0x00,0x64,0x0B,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x0B,0x18,0x09,0x38,0x0C,0x95,0x0C,0x7D,0x04,0x2F,0x05,0x49,0x00,0xEF,0x00,0xDA,0x01,0x04,0x05,0x11,0x00,0x50,0xA4,0x20};
      //for (uint8_t i = 0; i<sizeof(ReadBuffer2); i++) {
      //  this->DataFrame->push_back(ReadBuffer2[i]);
      //  if (Config->GetDebugLevel() >=4) {Serial.print(PrintHex(ReadBuffer2[i])); Serial.print(" ");}
      //}
      
      if (Config->GetDebugLevel() >=4) { Serial.println(); }
    #endif
    // ***********************************************
  }

  if (this->DataFrame->size() > 0) {
    
    // setup RequestType
    if (this->DataFrame->at(this->Conf_IdDataFunctionCodePos) == this->Conf_IdDataFunctionCode) {
      RequestType = "id";
    } else if (this->DataFrame->at(this->Conf_LiveDataFunctionCodePos) == this->Conf_LiveDataFunctionCode) {
      RequestType = "livedata";
    }

    ProgmemStream stream{JSON};
    String streamString = "";
    streamString = "\""+ this->InverterType +"\": {";
    stream.find(streamString.c_str());
    
    streamString = "\""+ RequestType +"\": [";
    stream.find(streamString.c_str());
    do {
      StaticJsonDocument<1024> elem;
      DeserializationError error = deserializeJson(elem, stream); 
      
      if (!error) {
        // Print the result
        if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
        if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);}
      } else {
        if (Config->GetDebugLevel() >=1) {
          Serial.print("(Function ParseData) Failed to parse JSON Register Data: "); 
          Serial.print(error.c_str()); 
          Serial.println();
        }
      }

      // setUp local variables
      String datatype = "";
      String openwbtopic = "";
      float factor = 1;
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
          sprintf(dbg, "Error: for Name '%s' no position array found", d.Name);
          Serial.println(dbg);
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
        val_f = (float)this->JsonPosArrayToInt(posArray, posArray2) * factor;
        sprintf(dbg, "%.2f", val_f);
        d.value = String(dbg);
      
      } else if (datatype == "integer") {
        //********** handle Datatype Integer ***********//
        val_i = this->JsonPosArrayToInt(posArray, posArray2) * factor;
        sprintf(dbg, "%d", val_i);
        d.value = String(dbg);

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
        sprintf(dbg, "Error: for Name '%s' no valid datatype found", d.Name.c_str());
        if (Config->GetDebugLevel() >=2) {Serial.println(dbg);}
      }


      // map values if a mapping is specified
      if(!elem["mapping"].isNull() && elem["mapping"].is<JsonArray>() && d.value != "") {
        sprintf(dbg, "Map values for item %s", d.Name.c_str());
        if (Config->GetDebugLevel() >=4) {Serial.println(dbg);}

        JsonArray map = elem["mapping"].as<JsonArray>();
        d.value = this->MapItem(map, d.value);
      }

      sprintf(dbg, "Data: %s -> %s %s", d.Name.c_str(), d.value.c_str(), d.unit.c_str());
      if (Config->GetDebugLevel() >=4) {Serial.println(dbg);}

      if (this->mqtt && IsActiveItem && d.Name != "") { 
          this->mqtt->Publish_String(d.Name.c_str(), d.value, false);
          if (openwbtopic.length() > 0) { this->mqtt->Publish_String(openwbtopic.c_str(), d.value, true);}
      }

      if(RequestType == "livedata") {
        this->ChangeRegItem(this->InverterLiveData, d);
      } else if(RequestType == "id") {
        this->ChangeRegItem(this->InverterIdData, d);
        
        if (Config->GetDebugLevel() >=3) {
          Serial.printf("Inverter ID Data found -> %s: %s \n", d.Name.c_str(), d.value.c_str());
        }

      }
    
    } while (stream.findUntil(",","]"));

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
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  String ret = value;

  for (JsonArray mapItem : map) {
    String v1 = mapItem[0].as<String>();
    String v2 = mapItem[1].as<String>();

    sprintf(dbg, "Check Map value: %s -> %s", v1.c_str(), v2.c_str());
    if (Config->GetDebugLevel() >=5) {Serial.println(dbg);}

    if (value == v1) {
      ret = v2;
      sprintf(dbg, "Mapped value: %s -> %s", v1.c_str(), v2.c_str());
      if (Config->GetDebugLevel() >=4) {Serial.println(dbg);}
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
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)message[pos];          // XOR byte into least sig. byte of crc
    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
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
 * {data: [{"name": "xx", "value": "xx"}], }
 * {"GridVoltage_R":"0.00 V","GridCurrent_R":"0.00 A","GridPower_R":"0 W","GridFrequency_R":"0.00 Hz","GridVoltage_S":"0.90 V","GridCurrent_S":"1715.40 A","GridPower_S":"-28671 W","GridFrequency_S":"174.08 Hz","GridVoltage_T":"0.00 V","GridCurrent_T":"0.00 A","GridPower_T":"0 W","GridFrequency_T":"1.30 Hz","PvVoltage1":"259.80 V","PvVoltage2":"0.00 V","PvCurrent1":"1.00 A","PvCurrent2":"0.00 A","Temperature":"28 &deg;C","PowerPv1":"283 W","PowerPv2":"0 W","BatVoltage":"0.00 V","BatCurrent":"0.00 A","BatPower":"0 W","BatTemp":"0 &deg;C","BatCapacity":"0 %","OutputEnergyChargeWh":"0 Wh","OutputEnergyChargeKWh":"0.00 KWh","OutputEnergyChargeToday":"0.00 KWh","InputEnergyChargeWh":"0 Wh","InputEnergyChargeKWh":"0.00 KWh"}
*******************************************************/
void modbus::GetLiveDataAsJson(AsyncResponseStream *response) {
  int count = 0;
  response->print("{\"data\": ["); 
  
  for (uint16_t i=0; i < this->InverterLiveData->size(); i++) {
    StaticJsonDocument<512> doc;
    String s = "";
    doc["name"]  = this->InverterLiveData->at(i).Name;
    doc["realname"]  = this->InverterLiveData->at(i).RealName;
    doc["value"] = this->InverterLiveData->at(i).value + " " + this->InverterLiveData->at(i).unit;
    doc["active"] = (this->InverterLiveData->at(i).active?1:0);
    doc["mqtttopic"] = this->mqtt->getTopic(this->InverterLiveData->at(i).Name, false);

    serializeJson(doc, s);
    if(count>0) response->print(", ");
    response->print(s);
    count++;
  }
  
  response->print(" ]}");
}

/*******************************************************
 * Return all LiveData as jsonArray
 * {data: [{"name": "xx", "value": "xx"}], }
 * {"GridVoltage_R":"0.00 V","GridCurrent_R":"0.00 A","GridPower_R":"0 W","GridFrequency_R":"0.00 Hz","GridVoltage_S":"0.90 V","GridCurrent_S":"1715.40 A","GridPower_S":"-28671 W","GridFrequency_S":"174.08 Hz","GridVoltage_T":"0.00 V","GridCurrent_T":"0.00 A","GridPower_T":"0 W","GridFrequency_T":"1.30 Hz","PvVoltage1":"259.80 V","PvVoltage2":"0.00 V","PvCurrent1":"1.00 A","PvCurrent2":"0.00 A","Temperature":"28 &deg;C","PowerPv1":"283 W","PowerPv2":"0 W","BatVoltage":"0.00 V","BatCurrent":"0.00 A","BatPower":"0 W","BatTemp":"0 &deg;C","BatCapacity":"0 %","OutputEnergyChargeWh":"0 Wh","OutputEnergyChargeKWh":"0.00 KWh","OutputEnergyChargeToday":"0.00 KWh","InputEnergyChargeWh":"0 Wh","InputEnergyChargeKWh":"0.00 KWh"}
*******************************************************/
void modbus::GetRegisterAsJson(AsyncResponseStream *response) {
  int count = 0;
  response->print("{\"data\": ["); 
  
  ProgmemStream stream{JSON};
  String streamString = "";
  streamString = "\""+ this->InverterType +"\": {";
  stream.find(streamString.c_str());

  streamString = "\"livedata\": [";
  stream.find(streamString.c_str());

  do {
    StaticJsonDocument<1024> elem;
    DeserializationError error = deserializeJson(elem, stream); 
      
    if (!error) {
      // Print the result
      if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
      if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);}
    } else {
      if (Config->GetDebugLevel() >=1) {
        Serial.print("(Function GetRegisterAsJson) Failed to parse JSON Register Data: "); 
        Serial.print(error.c_str()); 
        Serial.println();
      }
    }
  
    String s = "";
    serializeJson(elem, s);
    if(count>0) response->print(", ");
    response->print(s);
    count++;

  } while (stream.findUntil(",","]"));

  response->print("]}");
}

/*******************************************************
 * request for changing active Status of a certain item
 * Used by handleAjax function
*******************************************************/
void modbus::SetItemActiveStatus(String item, bool newstate) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  for (uint16_t j=0; j < this->InverterLiveData->size(); j++) {
    if (this->InverterLiveData->at(j).Name == item) {
      if (Config->GetDebugLevel() >=3) {
        sprintf(dbg, "Set Item <%s> ActiveState to %s", item.c_str(), (newstate?"true":"false"));
        Serial.println(dbg);
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
    
    this->QueryLiveData();
   }

  if (millis() - this->LastTxIdData > this->TxIntervalIdData * 1000) {
    this->LastTxIdData = millis();
    
    this->QueryIdData();
  }

  //its allowed to send a new request every 800ms, we use recommend 1000ms
  if (millis() - this->LastTxInverter > 1000) {
    this->LastTxInverter = millis();
    this->QueryQueueToInverter();
  }
}

/*******************************************************
 * save configuration from webfrontend into json file
*******************************************************/
void modbus::StoreJsonConfig(String* json) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, *json);
  
  if (error) { 
    if (Config->GetDebugLevel() >=1) {
      sprintf(dbg, "Cound not store jsonConfig completely -> %s", error.c_str());
      Serial.println(dbg);
    } 
  }

  JsonObject root = doc.as<JsonObject>();

  if (!root.isNull()) {
    File configFile = SPIFFS.open("/ModbusConfig.json", "w");
    if (!configFile) {
      if (Config->GetDebugLevel() >=0) {Serial.println("failed to open ModbusConfig.json file for writing");}
    } else {  
        serializeJsonPretty(doc, Serial);
        if (serializeJson(doc, configFile) == 0) {
          if (Config->GetDebugLevel() >=0) {Serial.println(F("Failed to write to file"));}
      }
      configFile.close();
    
      LoadJsonConfig(false);
    }
  }
}


/*******************************************************
 * save Item configuration from webfrontend into json file
*******************************************************/
void modbus::StoreJsonItemConfig(String* json) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  File configFile = SPIFFS.open("/ModbusItemConfig.json", "w");
  if (!configFile) {
    if (Config->GetDebugLevel() >=0) {Serial.println("failed to open ModbusItemConfig.json file for writing");}
  } else {  
    
    if (!configFile.print(*json)) {
        if (Config->GetDebugLevel() >=0) {Serial.println(F("Failed writing ItemConfig to file"));}
    }

    configFile.close();
  
    LoadJsonItemConfig();
  }
}

/*******************************************************
 * load initial Register Items from ProgMemStream into vector
*******************************************************/
void modbus::LoadRegItems(std::vector<reg_t>* vector, String type) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  vector->clear();

  if (Config->GetDebugLevel() >=4) {
    sprintf(dbg, "Load RegItems for Inverter %s and type <%s>", this->InverterType.c_str(), type);
    Serial.println(dbg);
  }

  ProgmemStream stream{JSON};
  String streamString = "";
  streamString = "\""+ this->InverterType +"\": {";
  stream.find(streamString.c_str());
    
  streamString = "\"" + type + "\": [";
  stream.find(streamString.c_str());
  do {
    StaticJsonDocument<1012> elem;
    DeserializationError error = deserializeJson(elem, stream); 
      
    if (!error) {
      // Print the result
      if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
      if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);}
    } else {
      if (Config->GetDebugLevel() >=1) {
        sprintf(dbg, "(Function LoadRegItems) Failed to parse JSON Register Data for Inverter <%s> and type <%s>: %s", this->InverterType.c_str(), type, error.c_str());
        Serial.println(dbg);
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

    d.active = false; // set initial
    vector->push_back(d);

    if (Config->GetDebugLevel() >=4) {
      sprintf(dbg, "processed RegItem: %s", d.Name.c_str());
      Serial.println(dbg);
    }

  } while (stream.findUntil(",","]"));
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
  String InverterType_old = this->InverterType;

  if (SPIFFS.exists("/ModbusConfig.json")) {
    //file exists, reading and loading
    if (Config->GetDebugLevel() >=3) Serial.println("reading config file....");
    File configFile = SPIFFS.open("/ModbusConfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) Serial.println("config file is open:");
      //size_t size = configFile.size();

      StaticJsonDocument<1024> doc; // TODO Use computed size??
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error) {
        if (Config->GetDebugLevel() >=3) { serializeJsonPretty(doc, Serial); Serial.println(); }
        
        if (doc.containsKey("pin_rx"))           { this->pin_RX = (int)(doc["pin_rx"]);} else {this->pin_RX = 16;}
        if (doc.containsKey("pin_tx"))           { this->pin_TX = (int)(doc["pin_tx"]);} else {this->pin_TX = 17;}
        if (doc.containsKey("pin_rts"))          { this->pin_RTS = (int)(doc["pin_rts"]);} else {this->pin_RTS = 18;}
        if (doc.containsKey("clientid"))         { this->ClientID = strtoul(doc["clientid"], NULL, 16);} else {this->ClientID = 0x01;} // hex convert to dec
        if (doc.containsKey("baudrate"))         { this->Baudrate = (int)(doc["baudrate"]);} else {this->Baudrate = 19200;}
        if (doc.containsKey("txintervallive"))   { this->TxIntervalLiveData = (int)(doc["txintervallive"]);} else {this->TxIntervalLiveData = 5;}
        if (doc.containsKey("txintervalid"))     { this->TxIntervalIdData = (int)(doc["txintervalid"]);} else {this->TxIntervalIdData = 3600;}
        if (doc.containsKey("invertertype"))     { this->InverterType = (doc["invertertype"]).as<String>();} else {this->InverterType = "Solax-X1";}
        if (doc.containsKey("enable_openwbtopic")){ this->Conf_EnableOpenWBTopic = (doc["enable_openwbtopic"]).as<bool>();} else { this->Conf_EnableOpenWBTopic = false; }
        if (doc.containsKey("enable_setters"))   { this->Conf_EnableSetters = (doc["enable_setters"]).as<bool>();} else { this->Conf_EnableSetters = false; }
      } else {
        if (Config->GetDebugLevel() >=1) {Serial.println("failed to load modbus json config, load default config");}
        loadDefaultConfig = true;
      }
    }
  } else {
    if (Config->GetDebugLevel() >=3) {Serial.println("ModbusConfig.json config File not exists, load default config");}
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    this->pin_RX = 16;
    this->pin_TX = 17;
    this->pin_RTS = 18;
    this->ClientID = 0x01;
    this->Baudrate = 19200;
    this->TxIntervalLiveData = 5;
    this->TxIntervalIdData = 3600;
    this->InverterType = "Solax-X1";
    this->Conf_EnableOpenWBTopic = false;
    this->Conf_EnableSetters = false;

    loadDefaultConfig = false; //set back
  }

  // ReInit if Baudrate was changed, not at firstrun!
  if(!firstrun && (
     (Baudrate_old != this->Baudrate) ||
     (pin_RX_old   != this->pin_RX)   ||
     (pin_TX_old   != this->pin_TX)   ||
     (pin_RTS_old  != this->pin_RTS)) ) { 
    
    this->init(false);
  }

  // ReInit if Invertertype was changed
  if(!firstrun && (
    InverterType_old != this->InverterType) ) { 
    
    this->init(false);
  }

}

/*******************************************************
 * load Modbus Item configuration from file
*******************************************************/
void modbus::LoadJsonItemConfig() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  if (SPIFFS.exists("/ModbusItemConfig.json")) {
    //file exists, reading and loading
    if (Config->GetDebugLevel() >=3) Serial.println("reading modbus item config file....");
    File configFile = SPIFFS.open("/ModbusItemConfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) Serial.println("modbus item config file is open:");

      ReadBufferingStream stream{configFile, 64};
      stream.find("\"data\":[");
      do {
        StaticJsonDocument<512> elem;
        DeserializationError error = deserializeJson(elem, stream); 

        if (!error) {
          // Print the result
          if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
          if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);}
        } else {
          if (Config->GetDebugLevel() >=1) {
            Serial.print("(Function LoadJsonItemConfig) Failed to parse JSON Register Data: "); 
            Serial.print(error.c_str()); 
            Serial.println();
          }
        }

        String ItemName = elem["name"].as<String>();
        ItemName = ItemName.substring(7, ItemName.length()); //Name: active_<ItemName>

        for(uint16_t i=0; i<this->InverterLiveData->size(); i++) {
          if (this->InverterLiveData->at(i).Name == ItemName ) {
            this->InverterLiveData->at(i).active = elem["value"].as<bool>();

            sprintf(dbg, "item %s -> %s", ItemName.c_str(), (this->InverterLiveData->at(i).active?"enabled":"disabled"));
            if (Config->GetDebugLevel() >=3) {Serial.println(dbg);}

            break;
          }
        }

      } while (stream.findUntil(",","]"));
    } else {
      if (Config->GetDebugLevel() >=1) {Serial.println("failed to load modbus item json config, load default config");}
    }
  } else {
    if (Config->GetDebugLevel() >=3) {Serial.println("ModbusItemConfig.json config File not exists, all items are active as default");}
  }
}


/*******************************************************************************************************
 * WebContent
*******************************************************************************************************/
void modbus::GetWebContentConfig(AsyncResponseStream *response) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  String html = "";

  response->print("<form id='DataForm'>\n");
  response->print("<table id='maintable' class='editorDemoTable'>\n");
  response->print("<thead>\n");
  response->print("<tr>\n");
  response->print("<td style='width: 250px;'>Name</td>\n");
  response->print("<td style='width: 200px;'>Wert</td>\n");
  response->print("</tr>\n");
  response->print("</thead>\n");
  response->print("<tbody>\n");

  response->print("<tr>\n");
  response->print("<td>Modbus RX-Pin (Default: 16)</td>\n");
  response->printf("<td><input min='0' max='255' id='GpioPin_RX'name='pin_rx' type='number' style='width: 6em' value='%d'/></td>\n", this->pin_RX);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Modbus TX-Pin (Default: 17)</td>\n");
  response->printf("<td><input min='0' max='255' id='GpioPin_TX'name='pin_tx' type='number' style='width: 6em' value='%d'/></td>\n", this->pin_TX);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Modbus Direction Control RTS-Pin (Default: 18)</td>\n");
  response->printf("<td><input min='0' max='255' id='GpioPin_RTS'name='pin_rts' type='number' style='width: 6em' value='%d'/></td>\n", this->pin_RTS);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Solax Modbus ClientID (in hex) (Default: 01)</td>\n");
  response->printf("<td><input maxlength='2' name='clientid' type='text' style='width: 6em' value='%02x'/></td>\n", this->ClientID);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Modbus Baudrate (Default: 19200)</td>\n");
  response->printf( "<td><input min='9600' max='115200' name='baudrate' type='number' style='width: 6em' value='%d'/></td>\n", this->Baudrate);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Interval for Live Datatransmission in sec (Default: 5)</td>\n");
  response->printf("<td><input min='2' max='3600' name='txintervallive' type='number' style='width: 6em' value='%d'/></td>\n", this->TxIntervalLiveData);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Interval for ID Datatransmission in sec (Default: 3600)</td>\n");
  response->printf("<td><input min='10' max='86400' name='txintervalid' type='number' style='width: 6em' value='%d'/></td>\n", this->TxIntervalIdData);
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Select Inverter Type (Default: Solax X1)</td>\n");
  response->print("<td> <select name='invertertype' size='1' style='width: 10em'>");
  for (uint8_t i=0; i< AvailableInverters->size(); i++) {
    response->printf("<option value='%s' %s>%s</option>\n", AvailableInverters->at(i).c_str(), (AvailableInverters->at(i)==this->InverterType?"selected":"") , AvailableInverters->at(i).c_str());
  }
  response->print("</td></tr>\n");

  response->print("<tr>\n");
  response->print("<td>Enable OpenWB Compatibility</td>\n");
  response->print("  <td>\n");
  response->print("    <div class='onoffswitch'>\n");
  response->printf("      <input type='checkbox' name='enable_openwbtopic' class='onoffswitch-checkbox' id='enable_openwbtopic' %s>\n", (this->Conf_EnableOpenWBTopic?"checked":""));
  response->print("      <label class='onoffswitch-label' for='enable_openwbtopic'>\n");
  response->print("        <span class='onoffswitch-inner'></span>\n");
  response->print("        <span class='onoffswitch-switch'></span>\n");
  response->print("      </label>\n");
  response->print("    </div>\n");
  response->print("  </td></tr>\n");

  response->print("<tr>\n");
  response->print("<td>Enable Set Commands over MQTT (security issue)</td>\n");
  response->print("  <td>\n");
  response->print("    <div class='onoffswitch'>\n");
  response->printf("      <input type='checkbox' name='enable_setters' class='onoffswitch-checkbox' id='enable_setters' %s>\n", (this->Conf_EnableSetters?"checked":""));
  response->print("      <label class='onoffswitch-label' for='enable_setters'>\n");
  response->print("        <span class='onoffswitch-inner'></span>\n");
  response->print("        <span class='onoffswitch-switch'></span>\n");
  response->print("      </label>\n");
  response->print("    </div>\n");
  response->print("  </td></tr>\n");

  response->print("</tbody>\n");
  response->print("</table>\n");

  response->print("</form>\n\n<br />\n");
  response->print("<form id='jsonform' action='StoreModbusConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  response->print("  <input type='text' id='json' name='json' />\n");
  response->print("  <input type='submit' value='Speichern' />\n");
  response->print("</form>\n\n");
}

void modbus::GetWebContentItemConfig(AsyncResponseStream *response) {
  response->println("<template id='NewRow'>");
  response->println("<tr>");
  response->println("  <td>");
  response->println("    <div class='onoffswitch'>");
  response->println("        <input type='checkbox' name='active_{name}' class='onoffswitch-checkbox' onclick='ChangeActiveStatus(this.id)' id='activeswitch_{name}' {active}>");
  response->println("        <label class='onoffswitch-label' for='activeswitch_{name}'>");
  response->println("        <span class='onoffswitch-inner'></span>");
  response->println("        <span class='onoffswitch-switch'></span>");
  response->println("      </label>");
  response->println("    </div>");
  response->println("  </td>");
  response->println("  <td>");
  response->println("    <dfn class='tooltip_simple'>{realname}");
  response->println("      <span role='tooltip_simple'>{mqtttopic}</span>");
  response->println("    </dfn>");
  response->println("  </td>");
  response->println("  <td style='text-align: center'>");
  response->println("    <template id='openwb'>");
  response->println("      <dfn class='tooltip'>&#9989;");
  response->println("        <span role='tooltip'>{openwbtopic}</span>");
  response->println("      </dfn>");
  response->println("    </template>");
  response->println("  </td>");
  response->println("  <td><div id='{name}'>{value}</div></td>");
  response->println("</tr>");
  response->println("</template>");

  response->println("<form id='DataForm'>");
  response->println("<table id='maintable' class='editorDemoTable'>");
  response->println("<thead>");
  response->println("<tr>");
  response->println("<td style='width: 25px;'>Active</td>");
  response->println("<td style='width: 250px;'>Name</td>");
  response->println("<td style='width: 80px;'>OpenWB</td>");
  response->println("<td style='width: 200px;'>Wert</td>");
  response->println("</tr>");
  response->println("</thead>");
  response->println("<tbody>");

  response->println("</tbody>");
  response->println("</table>");
  response->println("</form><br />");
  response->println("<form id='jsonform' action='StoreModbusItemConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\", 2)'>");
  response->println("  <input type='text' id='json' name='json' />");
  response->println("  <input type='submit' value='Speichern' />");
  response->println("</form>");

  response->println("<script language='javascript' type='text/javascript'>");
  response->println("  var url = '/getitems'");
  response->println("    fetch(url)");
  response->println("    .then(response => response.json())");
  response->println("    .then(json => FillItemConfig('#maintable', '#NewRow', 0, json.data));");
  response->println("</script>");

}

void modbus::GetWebContentRawData(AsyncResponseStream *response) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  String html = "";

  response->print("<form id='DataForm'>\n");
  response->print("<table id='maintable' class='editorDemoTable'>\n");
  response->print("<thead>\n");
  response->print("<tr>\n");
  response->print("<td style='width: 250px;'>Typ</td>\n");
  response->print("<td style='width: 400px;'>Raw Data</td>\n");
  response->print("</tr>\n");
  response->print("</thead>\n");
  response->print("<tbody>\n");

  response->print("<tr>\n");
  response->print("<td>RawData of ID-Data</td>\n");
  response->print("<td style='padding-left: 20px; width: 300px; vertical-align: middle; word-wrap: break-word; border-right: 1px solid transparent;'><span id='id_rawdata_org' style='display: none;'>\n");
  
  for (uint16_t i = 0; i < (this->SaveIdDataframe->size()); i++) {
    response->print(this->PrintHex(this->SaveIdDataframe->at(i)));
    response->print(" ");
  }

  response->print("</span><span id='id_rawdata'></span></td>\n");
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>RawData of Live-Data</td>\n");
  response->print("<td style='padding-left: 20px; width: 300px; vertical-align: middle; word-wrap: break-word; border-right: 1px solid transparent;'><span id='live_rawdata_org' style='display: none;'>\n");
  
  for (uint16_t i = 0; i < this->SaveLiveDataframe->size(); i++) {
    response->print(this->PrintHex(this->SaveLiveDataframe->at(i)));
    response->print(" ");
  }

  response->print("</span><span id='live_rawdata'></span></td>\n");
  response->print("</tr>\n");

  response->print("</tbody>\n");
  response->print("</table>\n");

  response->print("<p>\n");
  response->print("<table id='maintable' class='editorDemoTable'>\n");
  
  response->print("  <tr>\n");
  response->print("    <td style='width: 250px;'>Insert your positions (comma separated) to test</td>\n");
  response->print("    <td style='width: 400px;'><input id='positions' type='text' style='width: 12em' value='15,16,17,18'/><input  type='button' value='test it' onclick='check_rawdata()' /><span id='rawdata_result'></span></td>\n");
  response->print("  </tr>\n");
  response->print("  <tr>\n");
  response->print("    <td>Options</td>\n");
  response->print("    <td>\n");
  response->print("      <table>\n");
  response->print("        <tr>\n");
  response->print("          <td>\n");
  response->print("            <input type='radio' id='int' name='datatype' value='int' checked>\n");
  response->print("            <label for='int'>Integer or Float</label><br>\n");
  response->print("            <input type='radio' id='string' name='datatype' value='string'>\n");
  response->print("            <label for='string'>String</label> \n");
  response->print("          </td>\n");
  response->print("          <td>\n");
  response->print("            <input type='radio' id='id' name='rawdatatype' value='id_rawdata'>\n");
  response->print("            <label for='id'>ID-Data</label><br>\n");
  response->print("            <input type='radio' id='live' name='rawdatatype' value='live_rawdata' checked>\n");
  response->print("            <label for='live'>Live-Data</label><br>\n");
  response->print("          </td>\n");
  response->print("        </tr>\n");
  response->print("      </table>\n");
  response->print("    </td>\n");
  response->print("  </tr>\n");

  response->print("</tbody>\n");
  response->print("</table>\n");
  response->print("</form>\n\n<br />\n");

  response->print("<script type = 'text/javascript'>\n");
  response->print("	reset_rawdata('id_rawdata');\n");
  response->print("	reset_rawdata('live_rawdata');\n");
  response->print("</script>\n");
}