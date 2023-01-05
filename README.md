# Modbus-RTU to MQTT Gateway for Solar Inverter

[![license](https://img.shields.io/badge/Licence-GNU%20v3.0-green)](https://github.com/desktop/desktop/blob/master/LICENSE)
![ESP32 Architecture](https://img.shields.io/badge/Architecture-ESP32-blue)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/tobiasfaust/SolaxModbusGateway?include_prereleases&style=plastic)
![GitHub All Releases](https://img.shields.io/github/downloads/tobiasfaust/SolaxModbusGateway/total?style=plastic)


This project implements a Gatewayx for Solar Inverter with with Modbus-RTU communication to MQTT on ESP32 basis.  
Direkt Communication wit [OpenWB](https://openwb.de) is implemented.

### Supported Solar Inverter
Basically, all Inverters with Modbus RS485 RTU communication. 
Currently the following Inverters are with thier special registers integrated:
* Solax Hybrid X1 G4 Series
* Growatt SPH

If your Solar Inverter is not listed, feel free to add the special register simply, please check th [wiki page](configuration-register) or contact me by opening a [new issue](https://github.com/tobiasfaust/SolaxModbusGateway/issues) in github.

### What you need
* ESP32 NodeMCU
* MAX485 Module TTL Switch Schalter to RS-485 Module RS485 5V Modul (this can also handle 3.3V from ESP)

An ESP8266 is actually not sufficient, because Modbus communication works fail-free only with hardwareserial. ESP8266 has only one Hardwareserial port which is used by serial/debug output. ESP32 has 3 Hardewareserial ports and we use one of them. Another reason is available memory for such huge json definition or such large modbus answers.  
Please check also the wiki page, [how to wire the circuit](wiring-the-circuit).

### How to start
It´s recommend to start with one example to check wiring works correctly. The example request the inverter SN and if wiring is correct, the inverter will answer with his number.

<code>
request: 01 03 00 00 00 07 08 04

Response: 01 03 .....
</code>


There is also a page with all configured livedata items available. This page is refreshing every 5 seconds. Please check [Modbus Item Configuration page](configuration-modbusitems).


## please refer full documentation in our [Wiki](https://github.com/tobiasfaust/SolaxModbusGateway/wiki)
