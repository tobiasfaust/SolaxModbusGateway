Release 3.3.2:
  - new feature: add confirmation dialog for ESP reset
  - migrate from old ajax communication to standard websocket communication
  - new feature: show set topics in WebUI (thanks to @laszgar)
  - new feature: add "newUpdate available" info in WebUI header
  - new feature: add toggle buttons at ModbusItems WebUI to change all items at once (#96)
  - new Inverter: add Growatt-SPH-V124 register file (thanks to @StefanNouza) (#109)
  
Release 3.3.1:
  - new Feature: datatype "binary" now available for json register definitions (PR #115)
  - BugFix: fix null-terminationof string handling (#96)
  - new feature: support for OpenWB 2.0 Api (#100)
  - bugfix: fix esp crash for /getitems if using an huge register table (#76)
  - fix CORS Issue when download a stable release
  - bugfix: fix register id definition (#113)

Release 3.3.0:
  - new feature: WebSerial as remote serial output (#74)
  - new feature: configurable serial output pins
  - replace deprecated arduinojson library function "containsKey"
  - improvement: use serveStatic function with cache-control instead of custom handling
  - new feature: Authorization/authentication on the web (#80)
  - new feature: defining multiple requests for id-data (thanks to @Lazgar) (#89)
  - new feature: add web-installer
  - new feature: use ImprovWiFi, remove ESPWifiManager
  - add donation button for paid extensions
  - new feature: replace customOTA to custom ElegantOTA
  - move config files from root to subfolder /config
  - new feature: Backup/Restore of configfiles for OTA

  **Breaking changes** 
  From release 0.7 onwards, an access point will no longer be opened during an initial installation. The WiFi access data must be entered via the web installer. This is used for both initial installation and entry of WiFi access data. 
  When updating version 0.6 to 0.7, a new installation must also be carried out because the WiFi handling has been switched to the ImprovWiFi Library. 
  If version 0.7 is already installed on the ESP device, an OTA update is sufficient. See “Update” button at ESP-webinterface 

  Detailed instructions can be found in the [WiKi](https://github.com/tobiasfaust/SolaxModbusGateway/wiki)

Release 3.2.2:
  - new feature: GoodWe Support, by @TigerGrey (#58)
  - new feature: support for id and livedate on same functioncode, by @TigerGrey (#58)
  - new feature: CRC Check to validate response (#43), enabling in Modbusconfig
  - new feature: data length check to validate response (#43), enabling in Modbusconfig
  - new feature: reading of potential-free Relays on Inverter (#67), enabling in Modbusconfig, thanks to @Lazgar
  
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
   

