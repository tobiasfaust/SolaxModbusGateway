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
  ActiveItems         = new std::vector<itemconfig_t>{};
  AvailableInverters  = new std::vector<String>{};

  Conf_RequestLiveData= new std::vector<std::vector<byte>>{};
  Conf_RequestIdData  = new std::vector<byte>{};

  RequestQueue = new ArduinoQueue<std::vector<byte>>(5); // max 5 requests

  this->LoadJsonConfig();
  this->LoadJsonItemConfig();
  this->LoadInvertersFromJson();
  this->LoadInverterConfigFromJson();
  this->init();    
}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init() {
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

  // https://forum.arduino.cc/t/creating-serial-objects-within-a-library/697780/11
  RS485Serial = new HardwareSerial(1);
  RS485Serial->begin(this->Baudrate, SERIAL_8N1, this->pin_RX, this->pin_TX);

  //at first read ID Data
  this->QueryIdData();
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
      sprintf(dbg, "Inverter found: %s", kv.key().c_str());
      if (Config->GetDebugLevel() >=3) {Serial.println(dbg);}

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
  
  StaticJsonDocument<1024> doc;
  StaticJsonDocument<100> filter;

  filter[this->InverterType]["config"] = true;
    
  DeserializationError error = deserializeJson(doc, JSON, DeserializationOption::Filter(filter));

  if (error && Config->GetDebugLevel() >=1) {
    sprintf(dbg, "Error: unable to read configdata for inverter %s: %s", this->InverterType, error.c_str());
    Serial.println(dbg);
  } else {
    if (Config->GetDebugLevel() >=4) {
      sprintf(dbg, "Read config data for inverter %s", this->InverterType);
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

  if (this->RequestQueue->isEmpty()) {
    if (Config->GetDebugLevel() >=4) { Serial.println(this->PrintDataFrame(this->Conf_RequestIdData).c_str()); }
    this->RequestQueue->enqueue(*this->Conf_RequestIdData);
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

  if (this->RequestQueue->isEmpty()) {
    for (uint8_t i = 0; i < this->Conf_RequestLiveData->size(); i++) {
      if (Config->GetDebugLevel() >=4) { Serial.println(this->PrintDataFrame(&this->Conf_RequestLiveData->at(i)).c_str()); }
      this->RequestQueue->enqueue(this->Conf_RequestLiveData->at(i));
    }
  }
}

/*******************************************************
 * send 1 query from queue to inverter 
*******************************************************/
void modbus::QueryQueueToInverter() {
  if (!this->RequestQueue->isEmpty()) {
    if (Config->GetDebugLevel() >=3) { Serial.print("Request queue data from inverter: "); }
  
    digitalWrite(this->pin_RTS, RS485Receive);     // init Receive
    while (RS485Serial->available() > 0) { // read serial if any old data is available
      RS485Serial->read();
    }

    std::vector<byte> m = this->RequestQueue->dequeue();

    byte message[m.size() + 2] = {0x00}; // +2 Byte CRC
  
    for (uint8_t i; i < m.size(); i++) {
      message[i] = m.at(i);
    }

    uint16_t crc = this->Calc_CRC(message, sizeof(message)-2);
    message[sizeof(message)-1] = highByte(crc);
    message[sizeof(message)-2] = lowByte(crc);

    if (Config->GetDebugLevel() >=3) { Serial.println(this->PrintDataFrame(message, sizeof(message))); }

    digitalWrite(this->pin_RTS, RS485Transmit);     // init Transmit
    RS485Serial->write(message, sizeof(message));
    RS485Serial->flush();

    delay(100);
    this->ReceiveData();

    if (this->RequestQueue->isEmpty()) {
      this->ParseData();
    }
  }
}

/*******************************************************
 * Receive Data after Quering, put them into vector
*******************************************************/
void modbus::ReceiveData() {
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
  int val_i = 0;
  int val2_i = 0;
  int val_max = 0;

  // handle Main Position Array
  if (!posArray.isNull()){ 
    for(int v : posArray) {
      if (v < this->DataFrame->size()) { 
        val_i = (val_i << 8) | this->DataFrame->at(v); 
        val_max = (val_max << 8) | 0xFF;
      }
    }
  }

  // handle Position Array 2
  if (!posArray2.isNull()){ 
    for(int w : posArray2) {
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
      byte ReadBuffer[] = {0x01, 0x04, 0xA6, 0x08, 0xF4, 0x00, 0x0D, 0x01, 0x0D, 0x0A, 0x26, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x13, 0x8B, 0x00, 0x1C, 0x00, 0x02, 0x01, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE6, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x09, 0x17, 0x04, 0x56};
      //byte ReadBuffer[] = {0x01, 0x03, 0x28, 0x48, 0x34, 0x35, 0x30, 0x32, 0x41, 0x49, 0x34, 0x34, 0x35, 0x39, 0x30, 0x30, 0x35, 0x73, 0x6F, 0x6C, 0x61, 0x78, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x4A, 0xA0};
      for (uint8_t i = 0; i<sizeof(ReadBuffer); i++) {
        this->DataFrame->push_back(ReadBuffer[i]);
        if (Config->GetDebugLevel() >=4) {Serial.print(PrintHex(ReadBuffer[i])); Serial.print(" ");}
      }
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

    // clear old data
    if(RequestType == "livedata") {
      this->InverterLiveData->clear();
    } else if(RequestType == "id") {
      this->InverterIdData->clear();
    }

    ProgmemStream stream{JSON};
    String streamString = "";
    streamString = "\""+ this->InverterType +"\": {";
    stream.find(streamString.c_str());
    
    streamString = "\""+ RequestType +"\": [";
    stream.find(streamString.c_str());
    do {
      StaticJsonDocument<512> elem;
      DeserializationError error = deserializeJson(elem, stream); 
      
      if (!error) {
        // Print the result
        if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
        if (Config->GetDebugLevel() >=5) {serializeJsonPretty(elem, Serial);}
      } else {
        if (Config->GetDebugLevel() >=1) {
          Serial.print("Failed to parse JSON Register Data: "); 
          Serial.print(error.c_str()); 
          Serial.println();
        }
      }

      // setUp local variables
      String datatype = "";
      float factor = 0;
      JsonArray posArray, posArray2;
      float val_f = 0;
      int val_i = 0;
      //int val_max = 0; 
      String val_str = "";
      reg_t d = {};
      bool IsActiveItem = false;
      String openwbtopic = "";
      
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
      if(!elem["realname"].isNull()) {
        d.RealName = elem["realname"].as<String>();
      } else {
        d.RealName = d.Name;
      }

      // optional field
      if(!elem["openwbtopic"].isNull()) {
        openwbtopic = elem["openwbtopic"].as<String>();
      } else {
        openwbtopic = "";
      }
      
      // optional field
      if (elem.containsKey("factor")) {
        factor = elem["factor"];
      } else { factor = 1; }
      
      // optional field
      // check, if a extra position for a negative value was defined
      if (elem["position-"].is<JsonArray>()) {
        posArray2 = elem["position2"].as<JsonArray>();
      } 

      // check if active item to send out via mqtt
      for (uint16_t i=0; i < this->ActiveItems->size(); i++) {
        if (this->ActiveItems->at(i).Name == d.Name && this->ActiveItems->at(i).value) {
          IsActiveItem = true;
        }
      }
      
      // ************* processing data ******************
      if (datatype == "float") {
        //********** handle Datatype FLOAT ***********//
        val_f = (float)this->JsonPosArrayToInt(posArray, posArray2) * factor;
        sprintf(dbg, "%.2f %s", val_f, elem["unit"].as<String>());
        d.value = String(dbg);

        if (this->mqtt && IsActiveItem) { 
          this->mqtt->Publish_Float(d.Name.c_str(), val_f, false);
          if (openwbtopic.length() > 0) { this->mqtt->Publish_Float(openwbtopic.c_str(), val_f, true);}
        }
        sprintf(dbg, "Data: %s -> %s", d.RealName.c_str(), d.value.c_str());
      
      } else if (datatype == "integer") {
        //********** handle Datatype Integer ***********//
        val_i = this->JsonPosArrayToInt(posArray, posArray2) * factor;
        sprintf(dbg, "%d %s", val_i, elem["unit"].as<String>());
        d.value = String(dbg);

        if (this->mqtt && IsActiveItem) { 
          this->mqtt->Publish_Int(d.Name.c_str(), val_i, false);
          if (openwbtopic.length() > 0) { this->mqtt->Publish_Int(openwbtopic.c_str(), val_i, true);}
        }
        sprintf(dbg, "Data: %s -> %s", d.RealName.c_str(), d.value.c_str());
      
      } else if (datatype == "string") {
        //********** handle Datatype String ***********//
        if (!posArray.isNull()){ 
          for(int v : posArray) {
            val_str.concat(String((char)this->DataFrame->at(v)));
          }
        } 
        d.value = val_str;

        if (this->mqtt && IsActiveItem) { 
          this->mqtt->Publish_String(d.Name.c_str(), d.value, false);
          if (openwbtopic.length() > 0) { this->mqtt->Publish_String(openwbtopic.c_str(), d.value, true); }
        }
        sprintf(dbg, "Data: %s -> %s", d.RealName.c_str(), d.value.c_str());
      } else {
        //****************** sonst, leer *******************//
        d.value = "";
        sprintf(dbg, "Error: for Name '%s' no valid datatype found", d.Name.c_str());
      }

      if (Config->GetDebugLevel() >=4) {Serial.println(dbg);}

      if(RequestType == "livedata") {
        this->InverterLiveData->push_back(d);
      } else if(RequestType == "id") {
        this->InverterIdData->push_back(d);
      }
    
    } while (stream.findUntil(",","]"));

  } 

  // all data were process, clear Dataframe now for next query
  // save this for WebGUI first
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
      sn = this->InverterIdData->at(i).value;
  }
  return sn;
}

/*******************************************************
 * Return all LiveData as json
*******************************************************/
String modbus::GetLiveDataAsJson() {
  String sn;
  StaticJsonDocument<1024> doc;
  for (uint16_t i=0; i < this->InverterLiveData->size(); i++) {
    for (uint16_t j=0; j < this->ActiveItems->size(); j++) {
      if (this->InverterLiveData->at(i).Name == this->ActiveItems->at(j).Name && this->ActiveItems->at(j).value) {
        doc[this->InverterLiveData->at(i).Name] = this->InverterLiveData->at(i).value;
        break;
      }
    }
  }
  serializeJson(doc, sn);
  return sn;  
}

/*******************************************************
 * request for changing active Status of a certain item
 * Used by handleAjax function
*******************************************************/
void modbus::SetItemActiveStatus(String item, bool newstate) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  for (uint16_t j=0; j < this->ActiveItems->size(); j++) {
    if (this->ActiveItems->at(j).Name == item) {
      if (Config->GetDebugLevel() >=3) {
        sprintf(dbg, "Set Item <%s> ActiveState to %s", item.c_str(), (newstate?"true":"false"));
        Serial.println(dbg);
      }
      this->ActiveItems->at(j).value = newstate;
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
  
      LoadJsonConfig();
    }
  }
}


/*******************************************************
 * save Item configuration from webfrontend into json file
*******************************************************/
void modbus::StoreJsonItemConfig(String* json) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, *json);
  
  if (error) { 
    if (Config->GetDebugLevel() >=1) {
      sprintf(dbg, "Cound not store jsonItemConfig completely -> %s", error.c_str());
      Serial.println(dbg);
    } 
  }

  JsonObject root = doc.as<JsonObject>();

  if (!root.isNull()) {
    File configFile = SPIFFS.open("/ModbusItemConfig.json", "w");
    if (!configFile) {
      if (Config->GetDebugLevel() >=0) {Serial.println("failed to open ModbusItemConfig.json file for writing");}
    } else {  
      serializeJsonPretty(doc, Serial);
      if (serializeJson(doc, configFile) == 0) {
        if (Config->GetDebugLevel() >=0) {Serial.println(F("Failed ItemConfig to write to file"));}
      }
      configFile.close();
  
      LoadJsonItemConfig();
    }
  }
}

/*******************************************************
 * load configuration from file
*******************************************************/
void modbus::LoadJsonConfig() {
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
        if (doc.containsKey("openwbtopic"))      { this->Conf_EnableOpenWBTopic = (doc["openwbtopic"]).as<bool>();} else { this->Conf_EnableOpenWBTopic = false; }
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

    loadDefaultConfig = false; //set back
  }

  // ReInit if Baudrate was changed
  if((Baudrate_old != this->Baudrate) ||
     (pin_RX_old   != this->pin_RX)   ||
     (pin_TX_old   != this->pin_TX)   ||
     (pin_RTS_old  != this->pin_RTS)) { this->init();}

  // ReInit if Invertertype was changed
  if(InverterType_old != this->InverterType) { this->LoadInverterConfigFromJson(); }

}

/*******************************************************
 * load Modbus Item configuration from file
*******************************************************/
void modbus::LoadJsonItemConfig() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  this->ActiveItems->clear();

  if (SPIFFS.exists("/ModbusItemConfig.json")) {
    //file exists, reading and loading
    if (Config->GetDebugLevel() >=3) Serial.println("reading modbus item config file....");
    File configFile = SPIFFS.open("/ModbusItemConfig.json", "r");
    if (configFile) {
      if (Config->GetDebugLevel() >=3) Serial.println("modbus item config file is open:");
      //size_t size = configFile.size();

      StaticJsonDocument<1024> doc; // TODO Use computed size??
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error) {
        if (Config->GetDebugLevel() >=3) { serializeJsonPretty(doc, Serial); Serial.println(); }

        // https://arduinojson.org/v6/api/jsonobject/begin_end/
        JsonObject root = doc.as<JsonObject>();
        for (JsonPair kv : root) {
          itemconfig_t item = {};
          
          item.Name = kv.key().c_str();
          item.Name = item.Name.substring(7, item.Name.length()); //Name: active_<ItemName>
          item.value = kv.value().as<bool>();

          sprintf(dbg, "item %s -> %d", item.Name.c_str(), item.value);
          if (Config->GetDebugLevel() >=4) {Serial.println(dbg);}
          this->ActiveItems->push_back(item);
        } 
      
      } else {
        if (Config->GetDebugLevel() >=1) {Serial.println("failed to load modbus item json config, load default config");}
      }
    }
  } else {
    if (Config->GetDebugLevel() >=3) {Serial.println("ModbusItemConfig.json config File not exists, all items are active as default");}
  }
}

/*******************************************************
 * WebContent
*******************************************************/
void modbus::GetWebContentConfig(WM_WebServer* server) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  String html = "";

  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 250px;'>Name</td>\n");
  html.concat("<td style='width: 200px;'>Wert</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");

  server->sendContent(html.c_str()); html = "";

  html.concat("<tr>\n");
  html.concat("<td>Modbus RX-Pin (Default: 16)</td>\n");
  sprintf(buffer, "<td><input min='0' max='255' id='GpioPin_RX'name='pin_rx' type='number' style='width: 6em' value='%d'/></td>\n", this->pin_RX);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Modbus TX-Pin (Default: 17)</td>\n");
  sprintf(buffer, "<td><input min='0' max='255' id='GpioPin_TX'name='pin_tx' type='number' style='width: 6em' value='%d'/></td>\n", this->pin_TX);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Modbus Direction Control RTS-Pin (Default: 18)</td>\n");
  sprintf(buffer, "<td><input min='0' max='255' id='GpioPin_RTS'name='pin_rts' type='number' style='width: 6em' value='%d'/></td>\n", this->pin_RTS);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Solax Modbus ClientID (in hex) (Default: 01)</td>\n");
  sprintf(buffer, "<td><input maxlength='2' name='clientid' type='text' style='width: 6em' value='%02x'/></td>\n", this->ClientID);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Modbus Baudrate (Default: 19200)</td>\n");
  sprintf(buffer, "<td><input min='9600' max='115200' name='baudrate' type='number' style='width: 6em' value='%d'/></td>\n", this->Baudrate);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Interval for Live Datatransmission in sec (Default: 5)</td>\n");
  sprintf(buffer, "<td><input min='2' max='3600' name='txintervallive' type='number' style='width: 6em' value='%d'/></td>\n", this->TxIntervalLiveData);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Interval for ID Datatransmission in sec (Default: 3600)</td>\n");
  sprintf(buffer, "<td><input min='10' max='86400' name='txintervalid' type='number' style='width: 6em' value='%d'/></td>\n", this->TxIntervalIdData);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Select Inverter Type (Default: Solax X1)</td>\n");
  html.concat("<td> <select name='invertertype' size='1' style='width: 10em'>");
  for (uint8_t i=0; i< AvailableInverters->size(); i++) {
    sprintf(buffer, "<option value='%s' %s>%s</option>\n", AvailableInverters->at(i), (AvailableInverters->at(i)==this->InverterType?"selected":"") , AvailableInverters->at(i));
    html.concat(buffer);
  }
  html.concat("</td></tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("<tr>\n");
  html.concat("<td>Enable OpenWB Compatibility</td>\n");
  html.concat("  <td>\n");
  html.concat("    <div class='onoffswitch'>\n");
  sprintf(buffer, "      <input type='checkbox' name='openwbtopic' class='onoffswitch-checkbox' id='openwbtopic' %s>\n", (this->Conf_EnableOpenWBTopic?"checked":""));
  html.concat(buffer);
  html.concat("      <label class='onoffswitch-label' for='openwbtopic'>\n");
  html.concat("        <span class='onoffswitch-inner'></span>\n");
  html.concat("        <span class='onoffswitch-switch'></span>\n");
  html.concat("      </label>\n");
  html.concat("    </div>\n");
  html.concat("  </td></tr>\n");

  html.concat("</tbody>\n");
  html.concat("</table>\n");

  server->sendContent(html.c_str()); html = "";

  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreModbusConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");

  server->sendContent(html.c_str()); html = "";
}

void modbus::GetWebContentItemConfig(WM_WebServer* server) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  String html = "";

  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 25px;'>Active</td>\n");
  html.concat("<td style='width: 250px;'>Name</td>\n");
  html.concat("<td style='width: 200px;'>Wert</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");

  server->sendContent(html.c_str()); html = "";

  
  ProgmemStream stream{JSON};
  String streamString = "";
  streamString = "\""+ this->InverterType +"\": {";
  stream.find(streamString.c_str());  
  stream.find("\"livedata\": [");
  do {
    StaticJsonDocument<512> elem;
    DeserializationError error = deserializeJson(elem, stream); 
    uint16_t i = 0;
    bool IsActiveItem = false;
    String ItemValue = "";

    // lookup for the current right value from inverter
    for (i=0; i < this->InverterLiveData->size(); i++) {
      if (this->InverterLiveData->at(i).Name == elem["name"].as<String>()) {
        ItemValue = this->InverterLiveData->at(i).value;
        break;
      }
    }

    // lookup for the item if it´s active or not
    for (i=0; i < this->ActiveItems->size(); i++) {
      if (this->ActiveItems->at(i).Name == elem["name"].as<String>() && this->ActiveItems->at(i).value) {
        IsActiveItem = true;
        break;
      }
    }


    html.concat("<tr>\n");

    html.concat("  <td>\n");
    html.concat("    <div class='onoffswitch'>\n");
    sprintf(buffer, "      <input type='checkbox' name='active_%s' class='onoffswitch-checkbox' onclick='ChangeActiveStatus(this.id)' id='activeswitch_%s' %s>\n", elem["name"].as<String>().c_str(), elem["name"].as<String>().c_str(), (IsActiveItem?"checked":""));
    html.concat(buffer);
    sprintf(buffer, "      <label class='onoffswitch-label' for='activeswitch_%s'>\n", elem["name"].as<String>().c_str());
    html.concat(buffer);
    html.concat("        <span class='onoffswitch-inner'></span>\n");
    html.concat("        <span class='onoffswitch-switch'></span>\n");
    html.concat("      </label>\n");
    html.concat("    </div>\n");
    html.concat("  </td>\n");

    sprintf(buffer, "  <td>%s</td>\n", (elem["realname"].isNull()?elem["name"].as<String>().c_str():elem["realname"].as<String>().c_str()));
    html.concat(buffer);
    sprintf(buffer, "  <td><div id='%s'>%s</div></td>\n", elem["name"].as<String>().c_str(), ItemValue.c_str());

    html.concat(buffer);
    html.concat("</tr>\n");

    server->sendContent(html.c_str()); html = "";


  } while (stream.findUntil(",","]"));    
  
  html.concat("</tbody>\n");
  html.concat("</table>\n");
  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreModbusItemConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");

  server->sendContent(html.c_str()); html = "";
}

void modbus::GetWebContentRawData(WM_WebServer* server) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  String html = "";

  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 250px;'>Typ</td>\n");
  html.concat("<td style='width: 400px;'>Raw Data</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");

  html.concat("<tr>\n");
  html.concat("<td>RawData of ID-Data</td>\n");
  html.concat("<td style='padding-left: 20px; width: 300px; vertical-align: middle; word-wrap: break-word; border-right: 1px solid transparent;'><span>\n");
  
  for (uint16_t i = 0; i < (this->SaveIdDataframe->size()); i++) {
    html.concat(this->PrintHex(this->SaveIdDataframe->at(i)));
    html.concat(" ");
    if (html.length() > 2000) { server->sendContent(html.c_str()); html = ""; }
  }

  html.concat("</span></td>\n");
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("<tr>\n");
  html.concat("<td>RawData of Live-Data</td>\n");
  html.concat("<td style='padding-left: 20px; width: 300px; vertical-align: middle; word-wrap: break-word; border-right: 1px solid transparent;'><span>\n");
  
  for (uint16_t i = 0; i < this->SaveLiveDataframe->size(); i++) {
    html.concat(this->PrintHex(this->SaveLiveDataframe->at(i)));
    html.concat(" ");
    if (html.length() > 2000) { server->sendContent(html.c_str()); html = ""; }
  }

  html.concat("</span></td>\n");
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("</tbody>\n");
  html.concat("</table>\n");
  html.concat("</form>\n\n<br />\n");
  server->sendContent(html.c_str()); html = "";
}