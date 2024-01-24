#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#pragma once

#include <fstream>
#include <iostream>

#include <string.h>
#include <FS.h>
#include <LITTLEFS.h>

#ifdef ESP8266
  extern "C" {
      #include "user_interface.h"
  }
  
  //#include <ESP8266WebServer.h>
  //#include <ESP8266HTTPUpdateServer.h>
  //#include <ESP8266mDNS.h>
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif ESP32
//  #include "SPIFFS.h"
  #include <WiFi.h> 
  //#include <ESPmDNS.h>
  //#include <ESPHTTPUpdateServer.h>
  #include <AsyncTCP.h>
#endif

#include <Update.h>
#include <ESPAsyncWebServer.h>
//#include <ESPAsyncDNSServer.h>
#include <DNSServer.h>
