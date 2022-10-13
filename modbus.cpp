#include "modbus.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : Baudrate(19200), TxInterval(5), LastTx(0) {
  Serial.println("Initialize Modbus Class");  
  Serial.print("TxIntervall: ");  Serial.println(TxInterval);  

  InverterData = new std::vector<reg_t>{};
}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init(uint8_t clientid) {
  Serial.println("Init Modbus");
  this->ClientID = clientid;

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
  Serial.println("Query ID Data");
  
  while (Serial2.available() > 0) { // read serial if any old data is available
    Serial2.read();
  }
  
  Serial.print("Schreibe Daten: ");
  byte message[8] = {this->ClientID, 
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
  
  for(uint8_t i=0; i<sizeof(message); i++){
    Serial.print(PrintHex(message[i]).c_str());Serial.print(' ');
  }
  Serial.println("");
    
  Serial2.write(message, sizeof(message));
  Serial2.flush();
}

/*******************************************************
 * Query Live Data to Inverter
*******************************************************/
void modbus::QueryLiveData() {
  Serial.println("Query Live Data");
  
  while (Serial2.available() > 0) { // read serial if any old data is available
    Serial2.read();
  }
  
  Serial.print("Schreibe Daten: ");
  byte message[8] = {this->ClientID, 
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
  
  for(uint8_t i=0; i<sizeof(message); i++){
    Serial.print(PrintHex(message[i]).c_str());Serial.print(' ');
  }
  Serial.println("");
    
  Serial2.write(message, sizeof(message));
  Serial2.flush();
}

/*******************************************************
 * Receive Live Data after Quering
*******************************************************/
void modbus::ReceiveData() {
  std::vector<byte>DataFrame {};
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));

  Serial.println("Lese Daten: ");

// TEST ***********************************************
byte ReadBuffer[] = {0x01, 0x04, 0x18, 0x08, 0xE7, 0x00, 0x0C, 0x00, 0xEE, 0x0A, 0xD5, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x13, 0x85, 0x00, 0x1C, 0x00, 0x02, 0x00, 0xF8, 0x00, 0x00, 0x2E, 0x8F};
for (uint8_t i = 0; i<sizeof(ReadBuffer); i++) {
  DataFrame.push_back(ReadBuffer[i]);
  Serial.print(PrintHex(ReadBuffer[i])); Serial.print(" ");
}
// ***********************************************

  if (!Serial2.available() || true) {
    int i = 0;
    while(Serial2.available()) {
      byte d = Serial2.read();
      DataFrame.push_back(d);
      Serial.print(PrintHex(d)); Serial.print(" ");
      i++;
    }    
    Serial.println();
    
    if (DataFrame.size() > 5 && DataFrame.at(0) == this->ClientID && DataFrame.at(1) != 0x83 && DataFrame.at(1) != 0x84) {
      // Dataframe valid
      Serial.println("Dataframe valid");
      // clear old data
      InverterData->clear();

      StaticJsonDocument<2048> doc;
      StaticJsonDocument<100> filter;
      if (DataFrame.at(1) == 0x03) {
        filter["id"] = true;
      } else if (DataFrame.at(1) == 0x04) {
        filter["livedata"] = true;
      }
      deserializeJson(doc, JSON, DeserializationOption::Filter(filter));

      // Print the result
      //serializeJsonPretty(doc, Serial);

      // https://arduinojson.org/v6/api/jsonobject/begin_end/
      JsonObject root = doc.as<JsonObject>();
      JsonObject::iterator it = doc.as<JsonObject>().begin();
      const char* rootname = it->key().c_str();
      
      for (JsonObject elem : doc[rootname].as<JsonArray>()) {
        
        if (((int)elem["position"] | 0) + ((int)elem["length"] | 0) > (DataFrame.size())-5) { // clientID(1), FunctionCode(1), Length(1), CRC(2)
          Serial.println("Error:cannot read more than receiving string");
          continue;
        }
      
        reg_t d = {};
        d.MqttTopicName = elem["MqttTopicName"];
        d.RealName = elem["realname"] | "?";
        uint8_t pos = ((uint8_t)elem["position"] | 0) + 3;
        //const char* name = elem["datatype"];  
        
        if (elem["datatype"] == "float") {
          float f = ((DataFrame.at(pos) << 8) | DataFrame.at(pos +1));
          if (elem.containsKey("factor")) { f = f * (float)elem["factor"]; }
          d.value = &f;
        } else if (elem["datatype"] == "integer") {
          int f = ((DataFrame.at(pos) << 8) | DataFrame.at(pos +1));
          if (elem.containsKey("factor")) { f = f * (int)elem["factor"]; }
          d.value = &f;
        }

        sprintf(dbg, "LiveData: %s -> %.2f", d.RealName, *(float*)d.value);
        Serial.println(dbg);

        InverterData->push_back(d);
      }
      Serial.print("Anzahl Datensätze: "); Serial.println(InverterData->size());
    } else { Serial.println("fault response received"); }
    
  } else {
    Serial.println("No client response");
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
