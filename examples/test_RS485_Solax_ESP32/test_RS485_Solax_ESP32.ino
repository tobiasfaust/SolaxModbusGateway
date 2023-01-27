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
  Serial.begin(115200);
  Serial.println("Test RS485 Connection");
  
  // Start the Modbus serial Port
  Serial2.begin(19200, SERIAL_8N1, RX, TX);
  delay(1000);
  
  while(Serial2.available())
  {
    Serial.print(Serial2.read(), HEX);
    Serial.print(" ");
  }
  
  delay(1000);
}


uint16_t Calc_CRC(uint8_t* message, uint8_t len) {
   //Calc the raw_msg_data_byte CRC code

  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)message[pos];          // XOR byte into least sig.byte of crc
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

String Hex2String(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "0x%02X", num);
  return hexCar;
}

void loop() {

  byte request[] = {0x01,
                               0x03,
                               0x00,
                               0x00,
                               0x00,
                               0x07, // 0x0D - 14 stellen, Inverter + Modulname
                               0x00,
                               0x00
           }; //

  uint16_t crc = Calc_CRC(request, sizeof(request)-2);
  request[sizeof(request)-1] = highByte(crc);
  request[sizeof(request)-2] = lowByte(crc);

  Serial.println("Schreibe Daten ....");
  for(uint8_t i=0; i<sizeof(request); i++){
    Serial.print(Hex2String(request[i]));Serial.print(' ');
  }
  Serial.println("");
  
  digitalWrite(RTS_pin, RS485Transmit);
  Serial2.write(request, sizeof(request));
  Serial2.flush();
  digitalWrite(RTS_pin,RS485Receive);
  
  delay(100);
  Serial.println("Lese Daten ....");
  
  while(Serial2.available())
  {
    Serial.print(Serial2.read(), HEX);
    Serial.print(" ");
  }
  Serial.println("");
  delay(2000);
}
