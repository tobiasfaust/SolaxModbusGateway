/*
Script to test a RS485 connection

from an idea of
https://arduino.stackexchange.com/questions/62327/cannot-read-modbus-data-repetitively
https://www.cupidcontrols.com/2015/10/software-serial-modbus-master-over-rs485-transceiver/

*/

#include <Arduino.h>

#define RX         16   // UART2 RX
#define TX         17   // UART2 TX
#define RTS_pin    5    // RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW




void setup() {

  pinMode(RTS_pin, OUTPUT);
  digitalWrite(RTS_pin, RS485Receive);

  // Start the built-in serial port, for Serial Monitor
  dbg.begin(115200);
  dbg.println("Test RS485 Connection");
  
  // Start the Modbus serial Port
  Serial2.begin(19200, SERIAL_8N1, RX, TX);
  delay(1000);
  
  while(Serial2.available())
  {
    dbg.print(Serial2.read(), HEX);
    dbg.print(" ");
  }
  
  delay(1000);
}


uint16_t Calc_CRC(uint8_t* message, uint8_t len) {
  // Calc the raw_msg_data_byte CRC code
  // special CRC computation for Solax Mini
  
  uint8_t i;
  uint16_t checksum = 0;
  for (i = 0; i <= len; i++) {
    checksum = checksum + message[i];
  }
  return checksum;
}

String Hex2String(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "0x%02X", num);
  return hexCar;
}

void loop() {
  // request request the device info of Solax Mini
  // assume, your Solax Mini has modbus SlaveID 0x0A, change it to yours at position 6 !
  
  byte request[] = {0xAA, 0x55, 
                    0x01, 0x00,
                    0x00, 0x0A,
                    0x11,
                    0x03,
                    0x00,
                    0x00,
                    0x00
           }; //

  uint16_t crc = Calc_CRC(request, sizeof(request)-2);
  request[sizeof(request)-1] = highByte(crc);
  request[sizeof(request)-2] = lowByte(crc);

  dbg.println("Schreibe Daten ....");
  for(uint8_t i=0; i<sizeof(request); i++){
    dbg.print(Hex2String(request[i]));Serial.print(' ');
  }
  dbg.println("");
  
  digitalWrite(RTS_pin, RS485Transmit);
  Serial2.write(request, sizeof(request));
  Serial2.flush();
  digitalWrite(RTS_pin,RS485Receive);
  
  delay(100);
  dbg.println("Lese Daten ....");
  
  while(Serial2.available())
  {
    dbg.print(Serial2.read(), HEX);
    dbg.print(" ");
  }
  dbg.println("");
  delay(2000);
}
