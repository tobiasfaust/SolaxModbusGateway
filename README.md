# Solax Modbus-RTU to MQTT Gateway
This project implements a Gatewayx for Solax X1/X3 Inverter with Version G4 with Modbus-RTU communication to MQTT on ESP32 basis.
Please note: A Solax Inverter Version G3 don´t works because the modbus protocol has changed between G3 and G4. 

### What you need
* ESP32 NodeMCU
* MAX485 Module TTL Switch Schalter to RS-485 Module RS485 5V Modul (this can also handle 3.3V from ESP)

An ESP8266 is actually not sufficient, because Modbus communication works fail-free only with hardwareserial. ESP8266 has only one Hardwareserial port which is used by serial/debug output. ESP32 has 3 Hardewareserial ports and we use one of them. 

### How to start
It´s recommend to start with one example to check wiring works correctly. The example request the inverter SN and if wiring is correct, the inverter will answer with his number.

<code>
request: 01 03 00 00 00 07 08 04

Response: 01 03 .....
</code>

## important: Project under development, not buggy free, no guarantee of fail-free working
