#include "modbus.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : Baudrate(19200), TxInterval(5), LastTx(0) {
  InverterData = new std::vector<reg_t>{};
}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init(uint8_t clientid, uint32_t baudrate) {
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  sprintf(dbg, "Init Modbus to Client 0x%02X with %d Baud", clientid, baudrate);
  Serial.println(dbg);

  this->ClientID = clientid;
  this->Baudrate = baudrate;

  // Start the Modbus serial Port
  Serial2.begin(this->Baudrate);   
}

/*******************************************************
 * set Interval for Query Inverter
*******************************************************/
void modbus::setTxInterval(int TxInterval) {
  this->TxInterval = TxInterval;
}

/*******************************************************
 * set Baudrate for Modbus
*******************************************************/
void modbus::setBaudrate(int baudrate) {
  this->Baudrate = baudrate;
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
                               0x07,  // Anzahl Register LSB
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
                               0x0C,  // Anzahl Register LSB
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
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));
  
  if (Config->GetDebugLevel() >=4) {Serial.println("Lese Daten: ");}

// TEST ***********************************************
//byte ReadBuffer[] = {0x01, 0x04, 0x18, 0x08, 0xE7, 0x00, 0x0C, 0x00, 0xEE, 0x0A, 0xD5, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x13, 0x85, 0x00, 0x1C, 0x00, 0x02, 0x00, 0xF8, 0x00, 0x00, 0x2E, 0x8F};
//for (uint8_t i = 0; i<sizeof(ReadBuffer); i++) {
//  DataFrame.push_back(ReadBuffer[i]);
//  Serial.print(PrintHex(ReadBuffer[i])); Serial.print(" ");
//}
// ***********************************************

  if (Serial2.available()) {
    int i = 0;
    while(Serial2.available()) {
      byte d = Serial2.read();
      DataFrame.push_back(d);
      if (Config->GetDebugLevel() >=5) {Serial.print(PrintHex(d)); Serial.print(" ");}
      i++;
    }    
    if (Config->GetDebugLevel() >=5) {Serial.println();}
    
    if (DataFrame.size() > 5 && DataFrame.at(0) == this->ClientID && DataFrame.at(1) != 0x83 && DataFrame.at(1) != 0x84) {
      // Dataframe valid
      if (Config->GetDebugLevel() >=4) {Serial.println("Dataframe valid");}
      // clear old data
      InverterData->clear();

      StaticJsonDocument<2048> regjson;
      StaticJsonDocument<200> filter;

      if (DataFrame.at(1) == 0x03) {
        filter["id"] = true;
      } else if (DataFrame.at(1) == 0x04) {
        filter["livedata"] = true;
      }
    
      DeserializationError error = deserializeJson(regjson, JSON, DeserializationOption::Filter(filter));

      if (!error) {
        // Print the result
        if (Config->GetDebugLevel() >=4) {Serial.println("parsing JSON ok"); }
        if (Config->GetDebugLevel() >=5) {serializeJsonPretty(regjson, Serial);}
      } else {
        if (Config->GetDebugLevel() >=1) {Serial.print("Failed to parse JSON Register Data: "); Serial.print(error.c_str()); }
      }
      
      // https://arduinojson.org/v6/api/jsonobject/begin_end/
      JsonObject root = regjson.as<JsonObject>();
      JsonObject::iterator it = regjson.as<JsonObject>().begin();
      const char* rootname = it->key().c_str();
      
      for (JsonObject elem : regjson[rootname].as<JsonArray>()) {
        float val_f = 0;
        int val_i = 0;
        float factor = 0;
        reg_t d = {};
        
        if (((int)elem["position"] | 0) + ((int)elem["length"] | 0) > (DataFrame.size())-5) { // clientID(1), FunctionCode(1), Length(1), CRC(2)
          if (Config->GetDebugLevel() >=1) {Serial.println("Error:cannot read more than receiving string");}
          continue;
        }
        
        d.Name = elem["Name"];
        d.RealName = elem["realname"] | "?";
        uint8_t pos = ((uint8_t)elem["position"] | 0) + 3;
        
        if (elem.containsKey("factor")) {
          factor = elem["factor"];
        } else { factor = 1; }
        
        if (elem["datatype"] == "float") {
          val_f = (((DataFrame.at(pos) << 8) | DataFrame.at(pos +1)) * factor);
          d.value = &val_f;
          sprintf(dbg, "Data: %s -> %.2f", d.RealName, *(float*)d.value);
        } else if (elem["datatype"] == "integer") {
          val_i = (((DataFrame.at(pos) << 8) | DataFrame.at(pos +1)) * (int)factor);
          d.value = &val_i;
          sprintf(dbg, "Data: %s -> %d", d.RealName, *(int*)d.value);
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
 * Calcule CRC Checksum
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
  if (millis() - this->LastTx > this->TxInterval*1000) {
    this->LastTx = millis();
    
    this->QueryLiveData();
    delay(100);
    this->ReceiveData();
  }
}
