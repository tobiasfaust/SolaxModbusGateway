#include "modbus.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : Baudrate(19200), TxInterval(5), LastTx(0) {
  Serial.println("Initialize Modbus Class");  
  Serial.print("TxIntervall: ");  Serial.println(TxInterval);  
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
  bool err = false;
  String errMsg = "";
  uint8_t payload_len = 0x00;
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));


  Serial.println("Lese Daten: ");
  if (!Serial2.available()) {
    err =true;
    errMsg = "no Response from client";
  }
  
  if (!err && Serial2.available() && Serial2.read() != this->ClientID) {
    err =true;
    errMsg = "ClientID nicht erkannt";
  }

  if (!err && Serial2.available() && (Serial2.read() == 0x83 || Serial2.read() == 0x84)) {
    if (Serial2.available()) {
      errMsg = Serial2.read();
    }else {
      errMsg = "Slave fault response";}
      err =true;
  }

  if (!err && Serial2.available() ) {
    payload_len = Serial2.read();
    Serial.print("PayLoad Länge: ");Serial.println(payload_len);
  }

// TEST
payload_len=24;
uint8_t ReadBuffer[payload_len + 2] = {0x08, 0xE7, 0x00, 0x0C, 0x00, 0xEE, 0x0A, 0xD5, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x13, 0x85, 0x00, 0x1C, 0x00, 0x02, 0x00, 0xF8, 0x00, 0x00, 0x2E, 0x8F};
//  uint8_t ReadBuffer[payload_len + 2]; // + 2Byte CRC
  
  if (payload_len > 0) {
    int i = 0;
    while(Serial2.available()) {
      ReadBuffer[i] = Serial2.read();
      Serial.print(PrintHex(ReadBuffer[i])); Serial.print(" ");
      i++;
    }    
    Serial.println();

    // loop über den Buffer und zuordnung
    for (uint8_t i = 0; i < (sizeof(RegsLiveData)/sizeof(reg_t)); i++) {
      
      if (RegsLiveData[i].Position + RegsLiveData[i].Length > (sizeof(ReadBuffer)-1)) {
        Serial.println("Error:cannot read more than receiving string");
        continue;
      }

      if (RegsLiveData[i].Datatype == STRING) {  // String
        String val = "";
        for (uint8_t j = 0; j < RegsLiveData[i].Length; j++) {
          // String* val = static_cast<String*>(RegsLiveData[i].value); // -> zurücklesen
          val += (char)ReadBuffer[RegsLiveData[i].Position + j];
        }
        RegsLiveData[i].value = &val;
        sprintf(dbg, "String Ausgabe: %s", val.c_str());
        Serial.println(dbg);
      } else if (RegsLiveData[i].Datatype == INTEGER) {  // Integer
        float f = ((ReadBuffer[RegsLiveData[i].Position] << 8) | ReadBuffer[RegsLiveData[i].Position +1]) * RegsLiveData[i].factor;
        RegsLiveData[i].value = &f;
        sprintf(dbg, "RegsLiveData (%d): %s -> %.2f", i, RegsLiveData[i].RealName.c_str(), (*(float*)RegsLiveData[i].value));
        Serial.println(dbg);
      }
      
    }
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
String modbus::PrintHex(uint8_t num) {
  char hexCar[2];

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
