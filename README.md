# Modbus-RTU to MQTT Gateway for Solar Inverter

[![license](https://img.shields.io/badge/Licence-GNU%20v3.0-green)](https://github.com/desktop/desktop/blob/master/LICENSE)
![ESP32 Architecture](https://img.shields.io/badge/Architecture-ESP32-blue)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/tobiasfaust/SolaxModbusGateway?include_prereleases&style=plastic)
![GitHub All Releases](https://img.shields.io/github/downloads/tobiasfaust/SolaxModbusGateway/total?style=plastic)

-----
[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/donate?hosted_button_id=RYUQVSNNTP3N6)

If you think the project is great, I would be happy about a donation to be able to integrate paid extensions.
I want to integrate the professional WebSerial and professional ElegantOTA Edition but each of them costs 250€.

-----

This project implements a Gatewayx for Solar Inverter with with Modbus-RTU communication to MQTT on ESP32 basis.  
Direkt Communication wit [OpenWB](https://openwb.de) is implemented.

### Supported Solar Inverter
Basically, all Inverters with Modbus RS485 RTU communication. 
Currently the following Inverters are with thier special registers integrated:
* Solax Hybrid X1
* Solax Hybrid X3 
* Solax MIC
* Growatt SPH
* Sofar-KTL Solarmax-SGA
* Deye Sun SG04LP3
* Deye Sun SG05LP3
* QVolt-HYP-G3-3P

If your Solar Inverter is not listed, feel free to add the special register simply, please check th [wiki page](configuration-register) or contact me by opening a [new issue](https://github.com/tobiasfaust/SolaxModbusGateway/issues) in github.

### What you need
* ESP32 NodeMCU
* MAX485 Module TTL Switch Schalter to RS-485 Module RS485 5V Modul (this can also handle 3.3V from ESP)

An ESP8266 is actually not sufficient, because Modbus communication works fail-free only with hardwareserial. ESP8266 has only one Hardwareserial port which is used by serial/debug output. ESP32 has 3 Hardewareserial ports and we use one of them. Another reason is available memory for such huge json definition or such large modbus answers.  
Please check also the wiki page, [how to wire the circuit](wiring-the-circuit).

### How to start
It's the easiest way you've ever heard: Just go to the [Web-Installer](https://tobiasfaust.github.io/SolaxModbusGateway/) and follow the [Wiki documentation](https://github.com/tobiasfaust/SolaxModbusGateway/wiki/start-and-integration-into-locale-wifi)

After that your Device is ready and looks like this:

[[images/Solax_Status.png]]


There is also a page with all configured livedata items available. This page is refreshing every 5 seconds. Please check [Modbus Item Configuration page](configuration-modbusitems).
