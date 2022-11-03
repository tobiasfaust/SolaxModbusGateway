#include "modbus.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : Baudrate(19200), LastTxLiveData(0), LastTxIdData(0) {
  InverterData = new std::vector<reg_t>{};
  AvailableInverters = new std::vector<String>{};

  // https://forum.arduino.cc/t/creating-serial-objects-within-a-library/697780/11
  //HardwareSerial mySerial(2);
  mySerial = new HardwareSerial(2);

  this->LoadJsonConfig();
  this->LoadInvertersFromJson();
  this->LoadInverterConfigFromJson();
  this->init();
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

  filter["*"]["config"]["IdDataStartsAtPos"] = true;  
  
  DeserializationError error = deserializeJson(regjson, JSON, DeserializationOption::Filter(filter));

  if (!error) {
    serializeJsonPretty(regjson, Serial);
    
    // https://arduinojson.org/v6/api/jsonobject/begin_end/
    JsonObject root = regjson.as<JsonObject>();
    for (JsonPair kv : root) {
      sprintf(dbg, "Inverter found: %s", kv.key().c_str());
      if (Config->GetDebugLevel() >=3) {Serial.println(dbg);}

      AvailableInverters->push_back(kv.key().c_str());
    }
  }
}

/*******************************************************
 * read config data from JSON, part "config"
*******************************************************/
void modbus::LoadInverterConfigFromJson() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  StaticJsonDocument<512> IDjson;
  StaticJsonDocument<100> IDfilter;

  IDfilter[this->InverterType]["config"] = true;
    
  DeserializationError error = deserializeJson(IDjson, JSON, DeserializationOption::Filter(IDfilter));

  if (error && Config->GetDebugLevel() >=1) {
    sprintf(dbg, "Error: unable to read configdata for inverter: %s", this->InverterType);
    Serial.println(dbg);
  }
}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init() {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  sprintf(dbg, "Init Modbus to Client 0x%02X with %d Baud", this->ClientID, this->Baudrate);
  if (Config->GetDebugLevel() >=3) {Serial.println(dbg);}

  // Start the Modbus serial Port
  Serial2.begin(this->Baudrate); 
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
  if (Config->GetDebugLevel() >=4) {Serial.println("Query ID Data");}
  
  while (Serial2.available() > 0) { // read serial if any old data is available
    Serial2.read();
  }
  
  byte message[] = {this->ClientID, 
                               0x03,  // FunctionCode
                               0x00,  // StartAddress MSB
                               0x00,  // StartAddress LSB
                               0x00,  // Anzahl Register MSB
                               0x14,  // Anzahl Register LSB
                               0x00,  // CRC LSB
                               0x00   // CRC MSB
           }; // 
           
  uint16_t crc = this->Calc_CRC(message, sizeof(message)-2);
  message[sizeof(message)-1] = highByte(crc);
  message[sizeof(message)-2] = lowByte(crc);
    
  Serial2.write(message, sizeof(message));
  Serial2.flush();
}

/*******************************************************
 * Query Live Data to Inverter
*******************************************************/
void modbus::QueryLiveData() {
  if (Config->GetDebugLevel() >=4) {Serial.println("Query Live Data");}
  
  while (Serial2.available() > 0) { // read serial if any old data is available
    Serial2.read();
  }
  
  byte message[] = {this->ClientID, 
                               0x04,  // FunctionCode
                               0x00,  // StartAddress MSB
                               0x00,  // StartAddress LSB
                               0x00,  // Anzahl Register MSB
                               0x23,  // Anzahl Register LSB
                               0x00,  // CRC LSB
                               0x00   // CRC MSB
           }; // 
           
  uint16_t crc = this->Calc_CRC(message, sizeof(message)-2);
  message[sizeof(message)-1] = highByte(crc);
  message[sizeof(message)-2] = lowByte(crc);
    
  Serial2.write(message, sizeof(message));
  Serial2.flush();
}

/*******************************************************
 * Receive Data after Quering
*******************************************************/
void modbus::ReceiveData() {
  std::vector<byte>DataFrame {};
  String RequestType = "";
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  if (Config->GetDebugLevel() >=4) {Serial.println("Lese Daten: ");}

// TEST ***********************************************
//byte ReadBuffer[] = {0x01, 0x04, 0x18, 0x08, 0xE7, 0x00, 0x0C, 0x00, 0xEE, 0x0A, 0xD5, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x13, 0x85, 0x00, 0x1C, 0x00, 0x02, 0x00, 0xF8, 0x00, 0x00, 0x2E, 0x8F};
//byte ReadBuffer[] = {0x01 ,0x04 ,0x46 ,0x09 ,0x01 ,0x00 ,0x0E ,0x01 ,0x21 ,0x0A ,0xC5 ,0x00 ,0x00 ,0x00 ,0x0A ,0x00 ,0x00 ,0x13 ,0x85 ,0x00 ,0x1D ,0x00 ,0x02 ,0x01 ,0x30 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xFE ,0xF6};
//for (uint8_t i = 0; i<sizeof(ReadBuffer); i++) {
//  DataFrame.push_back(ReadBuffer[i]);
//  if (Config->GetDebugLevel() >=4) {Serial.print(PrintHex(ReadBuffer[i])); Serial.print(" ");}
//}
// ***********************************************

  if (Serial2.available()) {
    int i = 0;
    // lese alle Daten, speichern im Vektor "Dataframe"
    while(Serial2.available()) {
      byte d = Serial2.read();
      DataFrame.push_back(d);
      if (Config->GetDebugLevel() >=5) {Serial.print(PrintHex(d)); Serial.print(" ");}
      i++;
    }    
    if (Config->GetDebugLevel() >=5) {Serial.println();}
    
    if (DataFrame.size() > 5 && 
        DataFrame.at(0) == this->ClientID && 
        DataFrame.at(1) != 0x83 && 
        DataFrame.at(1) != 0x84) {
      // Dataframe valid
      if (Config->GetDebugLevel() >=4) {Serial.println("Dataframe valid");}
      // clear old data
      InverterData->clear();

      StaticJsonDocument<2048> regjson;
      StaticJsonDocument<200> filter;

      if (DataFrame.at(1) == 0x03) {
        RequestType = "id";
      } else if (DataFrame.at(1) == 0x04) {
        RequestType = "livedata";
      }
      filter[this->InverterType]["data"][RequestType] = true;
    
      DeserializationError error = deserializeJson(regjson, JSON, DeserializationOption::Filter(filter));

      if (!error) {
        // Print the result
        if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
        if (Config->GetDebugLevel() >=5) {serializeJsonPretty(regjson, Serial);}
      } else {
        if (Config->GetDebugLevel() >=1) {Serial.print("Failed to parse JSON Register Data: "); Serial.print(error.c_str()); }
      }
      
      // https://arduinojson.org/v6/api/jsonobject/begin_end/
      //JsonObject root = regjson.as<JsonObject>();
      //JsonObject::iterator it = regjson.as<JsonObject>().begin();
      //const char* rootname = it->key().c_str();
      
      // über alle Elemente des JSON Arrays
      for (JsonObject elem : regjson[this->InverterType]["data"][RequestType].as<JsonArray>()) {
        float val_f = 0;
        int val_i = 0;
        String val_str = "";
        float factor = 0;
        JsonArray posArray;
        reg_t d = {};
        
        // mandantory field
        d.Name = elem["name"] | "undefined";
        
        // optional field
        if(!elem["realname"].isNull()) {
          d.RealName = elem["realname"];
        } else {
          d.RealName = d.Name;
        }
        
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

        // optional field
        if (elem.containsKey("factor")) {
          factor = elem["factor"];
        } else { factor = 1; }
        
        
        if (elem["datatype"] == "float") {
          //********** handle Datatype FLOAT ***********//
          if (!posArray.isNull()){ 
            for(int v : posArray) {
              if (v < DataFrame.size()-4) { val_i = (val_i << 8) | DataFrame.at(v +3); }
            }
          } 
          val_f = (float)val_i * factor;
          d.value = &val_f;
          if (this->mqtt) { this->mqtt->Publish_Float(d.Name, val_f);}
          sprintf(dbg, "Data: %s -> %.2f", d.RealName, *(float*)d.value);
        
        } else if (elem["datatype"] == "integer") {
          //********** handle Datatype Integer ***********//
          if (!posArray.isNull()){ 
            for(int v : posArray) {
              if (v < DataFrame.size()-4) { val_i = (val_i << 8) | DataFrame.at(v +3); }
            }
          } 
          val_i = val_i * factor;
          d.value = &val_i;
          if (this->mqtt) { this->mqtt->Publish_Int(d.Name, val_i);}
          sprintf(dbg, "Data: %s -> %d", d.RealName, *(int*)d.value);
        
        } else if (elem["datatype"] == "string") {
          //********** handle Datatype String ***********//
          if (!posArray.isNull()){ 
            for(int v : posArray) {
              val_str.concat(String((char)DataFrame.at(v +3)));
            }
          } 
          d.value = &val_str;
          if (this->mqtt) { this->mqtt->Publish_String(d.Name, val_str);}
          sprintf(dbg, "Data: %s -> %s", d.RealName, (*(String*)d.value).c_str());
        } else {
          d.value = NULL;
          sprintf(dbg, "Error: for Name '%s' no valid datatype found", d.Name);
        }

        if (Config->GetDebugLevel() >=4) {Serial.println(dbg);}

        InverterData->push_back(d);
      }
    } else { 
      if (Config->GetDebugLevel() >=3) {Serial.println("unexpected response received: ");} 
      this->PrintDataFrame(&DataFrame);
    }
  } else {
    if (Config->GetDebugLevel() >=3) {Serial.println("No client response");}
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
  char hexCar[4];
  sprintf(hexCar, "0x%02X", num);
  return hexCar;
}

/*******************************************************
 * friendly output the entire received Dataframe
*******************************************************/
String modbus::PrintDataFrame(std::vector<byte>* frame) {
  String out = "";
  for (uint8_t i = 0; i<frame->size(); i++) {
    out.concat(this->PrintHex(frame->at(i)));
    out.concat(" ");
  }
  return out;
}

/*******************************************************
 * Loop function
*******************************************************/
void modbus::loop() {
  if (millis() - this->LastTxLiveData > this->TxIntervalLiveData * 1000) {
    this->LastTxLiveData = millis();
    
    this->QueryLiveData();
    delay(100);
    this->ReceiveData();
  }

  if (millis() - this->LastTxIdData > this->TxIntervalIdData * 1000) {
    this->LastTxIdData = millis();
    
    this->QueryIdData();
    delay(100);
    this->ReceiveData();
  }
}

/*******************************************************
 * save configuration from webfrontend into json file
*******************************************************/
void modbus::StoreJsonConfig(String* json) {
  StaticJsonDocument<512> doc;
  deserializeJson(doc, *json);
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
 * load configuration from file
*******************************************************/
void modbus::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  uint32_t Baudrate_old = this->Baudrate;
  String InverterType_old = this->InverterType;


  if (SPIFFS.exists("/ModbusConfig.json")) {
    //file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open("/ModbusConfig.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      //size_t size = configFile.size();

      StaticJsonDocument<512> doc; // TODO Use computed size??
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error) {
        serializeJsonPretty(doc, Serial);
        
        if (doc.containsKey("clientid"))         { this->ClientID = strtoul(doc["clientid"], NULL, 16);} else {this->ClientID = 0x01;} // hex convert to dec
        if (doc.containsKey("baudrate"))         { this->Baudrate = (int)(doc["baudrate"]);} else {this->Baudrate = 19200;}
        if (doc.containsKey("txintervallive"))   { this->TxIntervalLiveData = (int)(doc["txintervallive"]);} else {this->TxIntervalLiveData = 5;}
        if (doc.containsKey("txintervalid"))     { this->TxIntervalIdData = (int)(doc["txintervalid"]);} else {this->TxIntervalIdData = 3600;}
        if (doc.containsKey("invertertype"))     { this->InverterType = (doc["invertertype"]).as<String>();} else {this->InverterType = "Solax-X1";}
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
    this->ClientID = 0x01;
    this->Baudrate = 19200;
    this->TxIntervalLiveData = 5;
    this->TxIntervalIdData = 3600;
    this->InverterType = "Solax-X1";
    
    loadDefaultConfig = false; //set back

    // ReInit if Baudrate was changed
    if(Baudrate_old != this->Baudrate) { this->init();}

    // ReInit if Invertertype was changed
    if(InverterType_old != this->InverterType) { this->LoadInverterConfigFromJson(); }
  }

}

/*******************************************************
 * WebContent
*******************************************************/
void modbus::GetWebContent(WM_WebServer* server) {
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
  
  html.concat("</tbody>\n");
  html.concat("</table>\n");


  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreModbusConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");

  server->sendContent(html.c_str()); html = "";
}
