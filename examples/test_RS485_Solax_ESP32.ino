/*
Anemometer with a RS485 wind sensor

from an idea of https://arduino.stackexchange.com/questions/62327/cannot-read-modbus-data-repetitively
https://www.cupidcontrols.com/2015/10/software-serial-modbus-master-over-rs485-transceiver/

_________________________________________________________________
|                                                               |
|       author : Philippe de Craene <dcphilippe@yahoo.fr        |
|       Any feedback is welcome                                 |
                                                                |
_________________________________________________________________

Materials :
• 1* Arduino Uno R3 - tested with IDE version 1.8.7 and 1.8.9
• 1* wind sensor - RS485 MODBUS protocol of communication
• 1* MAX485 DIP8

Versions chronology:
version 1 - 7 sept  2019   - first test 

*/

//#include <SoftwareSerial.h>  // https://github.com/PaulStoffregen/SoftwareSerial

//#define RX       17      // D1 =5  // 10    //Serial Receive pin
//#define TX      16      //  D2 = 4  //Serial Transmit pin
//#define RTS_pin    9    //RS485 Direction control
//#define RS485Transmit    HIGH
//#define RS485Receive     LOW

//SoftwareSerial RS485Serial(RX, TX); -> Arduino Mega = Serial2
 
void setup() {
   
//  pinMode(RTS_pin, OUTPUT);  
  
  // Start the built-in serial port, for Serial Monitor
  Serial.begin(115200);
  Serial.println("Anemometer"); 
  Serial.println("Anemometer"); 
  Serial.println("Anemometer"); 

  // Start the Modbus serial Port, for anemometer
  Serial2.begin(19200);   
  //RS485Serial.begin(19200);
  delay(1000);
}


uint16_t Calc_CRC(uint8_t* message, uint8_t len) {
   //Calc the raw_msg_data_byte CRC code
  
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

String Hex2String(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "0x%02X", num);
  return hexCar;
}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.println("Schreibe Daten ....");
  
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
  
  for(uint8_t i=0; i<sizeof(request); i++){
    Serial.print(Hex2String(request[i]));Serial.print(' ');
  }
  Serial.println("");
    
  Serial2.write(request, sizeof(request));
  Serial2.flush();
  
  delay(100);

/*  if (Serial2.available()) {
    byte Anemometer_buf[19];
    Serial2.readBytes(Anemometer_buf, 19);
  
    Serial.print("Response : ");
    for( byte i=0; i<18; i++ ) {
      Serial.print(Anemometer_buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
  } else {
    Serial.println("keine Daten da");
  }
 */

  Serial.println("Lese Daten ....");
  while(Serial2.available()) 
  {
    Serial.print(Serial2.read(), HEX);
    Serial.print(" ");
  }
 
  delay(1000);
}
