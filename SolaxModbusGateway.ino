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

*/
#pragma once

#include "commonlibs.h"
#include "modbus.h" 

void setup() {
   
  // Start the built-in serial port, for Serial Monitor
  Serial.begin(115200);
  Serial.println("Anemometer"); 
  Serial.println("Anemometer"); 
  Serial.println("Anemometer"); 

}




void loop() {
  // put your main code here, to run repeatedly:

  delay(1000);
}
