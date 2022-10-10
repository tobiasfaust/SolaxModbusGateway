
#include "commonlibs.h"
#include "modbus.h"
#include "register.h"

/*******************************************************
 * Constructor
*******************************************************/
modbus::modbus() : Baudrate(19200), TxInterval(5000) {}

/*******************************************************
 * initialize transmission
*******************************************************/
void modbus::init(uint8_t clientid) {
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
 * Query Live Data to Inverter
*******************************************************/
void modbus::QueryLiveData() {

  while (Serial2.available() > 0) { // read serial if any old data is available
    Serial2.read();
  }
  
  Serial.print("Schreibe Daten: ");
  byte message[8] = {this->ClientID, 
                               0x04,  // FunctionCode
                               0x00,  // StartAddress MSB
                               0x00,  // StartAddress LSB
                               0x00,  // Anzahl Register MSB
                               0x08,  // Anzahl Register LSB
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
  
  delay(100);
}

/*******************************************************
 * Receive Live Data after Quering
*******************************************************/
void modbus::ReceiveLiveData() {
  uint8_t buf = 0x00;
  bool err = false;
  String errMsg = "";
  uint8_t payload_len = 0x00;
  char dbg[100] = {0}; 
  memset(dbg, 0, sizeof(dbg));


  Serial.print("Lese Daten: ");
  if (!Serial2.available()) {
    err =true;
    errMsg = "no Response from client";
  }
  
  if (!err && Serial2.available() && Serial2.read() != this->ClientID) {
    err =true;
    errMsg = "ClientID nicht erkannt";
  }

  if (!err && Serial2.available() && Serial2.read() == 0x83) {
    if (Serial2.available()) {
      errMsg = Serial2.read();
    }else {errMsg = "undefined";}
    err =true;
  }

  if (!err && Serial2.available() ) {
    payload_len = Serial2.read();
  }
  
  if (payload_len > 0) {
    int i = 0;
    uint8_t ReadBuffer[payload_len];
  
    while(Serial2.available()) {
      ReadBuffer[i] = Serial2.read();
      Serial.print(ReadBuffer[i], HEX);
      Serial.println("");

      i++;
    }    
      
    // loop über den Buffer und zuordnung
    // regs[0].StartByte = 0;
    for (uint8_t i = 0; i<sizeof(regs); i++) {
      if (regs[i].Datatype == "S") {  // String
        String val = "";
        for (uint8_t j = 0; j < regs[i].Length; j++) {
          // String* val = static_cast<String*>(regs[i].value); // -> zurücklesen
          val += (char)ReadBuffer[regs[i].StartByte + j];
        }
        regs[i].value = val.c_str();
        sprintf(dbg, "String Ausgabe: %s", val.c_str());
        Serial.println(dbg);
      } else if (regs[i].Datatype == "I") {  // Integer
        regs[i].value = ReadBuffer[regs[i].StartByte];
        sprintf(dbg, "Integer Ausgabe: %d", ReadBuffer[regs[i].StartByte]);
        //Serial.printf( 1);
      }
      
    }
  }

}

/*******************************************************
 * Calcule CRC Checksum
*******************************************************/
uint16_t Calc_CRC(uint8_t* message, uint8_t len) {
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
    this->ReceiveLiveData();
  }
}
