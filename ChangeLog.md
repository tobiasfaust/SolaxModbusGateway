Release 3.2.3:
  - add WebSerial as remote serial output as its own variant (#74)
  - add: configurable serial output pins
  - replace deprecated arduinojson library "containsKey"
  - use serveStatic function instead of custom handling

Release 3.2.2:
  - add GoodWe Support, by @TigerGrey (#58)
  - add support for id and livedate on same functioncode, by @TigerGrey (#58)
  - add CRC Check to validate response (#43), enabling in Modbusconfig
  - add data length check to validate response (#43), enabling in Modbusconfig
  - add reading of potential-free Relays on Inverter (#67), enabling in Modbusconfig, thanks to @Lazgar
  
Release 3.2.1:
  - add modbusprotocoll doc for Solax-X1/X3 G4 (incl. write register) (thanks to @lattemacchiato) (#64)
  - fix bug in solax-x1.json at EnergyTotalToGrid
  - add Solax-Mic-Pro configuration, thanks to @michacee (#35)
  - add support for Growatt-MOD_4000_TL3-XH, thanks to @Gustavw82 (#70).
  - add page loading visualization
  
Release 3.2.0:
  - move register.h from PROGMEM into seperate files at FS
  - change deprecated SPIFFS to LittleFS
  - move all webfiles (css,js) to FS
  - create new Webpage to maintain the FS-files, editing json registers on-the-fly is now possible 
  - Add Solax-X3-Pro (thanks to @MagicSven81 - #30)
  - add WIFI event listener
  - add ethernet support for WT32_ETH01 (#26)
  - fix cutted ipaddress (#33)
  - extract html-code into separate html-files, so everyone can customize his own instance
  - interact with Web-frontend by json
  - enable update filesystem at updatepage
  - saving configs by global upload function, no extra saveconfigfile functions anymore (#57)
  - data partition is now larger
  - add device: QVolt-HYP-G3-3P
  
Release 3.1.1:
  - fix auto reconnect to WIFI (issue #21)
  - fix insecure unencrypted AccesspointMode
  - fix crash of ESP32 by using Solax-X3 settings (issue #22)
  - add support for ESP32-S2, -S3, -C3 (issue #20)
  - add optional register field: "valueAdd" to add/sub a static value (issue #32)
  - add "object_id" to jsondata of /getitems (issue #36)
  - add registers of Deye SUN-xx-SG04LP3 (issue #32)
  
Release 3.1.0:  
  - add Sofar-KTL Solarmax-SGA
  - bugfix: memoryleak
  - add a "mapping" for items
  - SOLAX-MIC added
  - bugfix: modbusitems sometime truncated
  - add official Releases Info

Release 3.0.0:  
  actual stable release

  - Solax X1
  - Solax X3
  - Growatt SPH

Release 2.0.0:  
  latest release, full working with Solax X1
  Solax X3 needs to configure in register.h. If you done that, feel free to make a pull request

Release 1.0.0
  first runnable version:
  - only with livedata
  - configurable data in webfrontend
  - Transmission Interval
  - Modbus Client-ID
  - Modbus Baudrate
  - Wifi Credentials via Wifimanager
  - MQTT Server, Port and optional Credentials
  - MQTT Basepath and Hostname

 Status Overview at webpage
  - IPAdresse
  - WiFiName
  - Wifi RSSI
  - MQTT Status
  - Uptime
  - Free Memory

 And Buttons to:
  - Reboot
  - Reset
  - WiFi Credentials Reset
   

