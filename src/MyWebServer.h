// https://github.com/esp8266/Arduino/issues/3205
// https://github.com/Hieromon/PageBuilder
// https://www.mediaevent.de/tutorial/sonderzeichen.html
//
// https://byte-style.de/2018/01/automatische-updates-fuer-microcontroller-mit-gitlab-und-platformio/
// https://community.blynk.cc/t/self-updating-from-web-server-http-ota-firmware-for-esp8266-and-esp32/18544
// https://forum.fhem.de/index.php?topic=50628.0

#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include "CommonLibs.h"
#include <ArduinoJson.h>
#include "uptime.h" // https://github.com/YiannisBourkelis/Uptime-Library/

#include "BaseConfig.h"
#include "modbus.h"
#include "mqtt.h"
#include "JavaScript.h"
#include "JsAjax.h"
#include "CSS.h"
#include "favicon.h"

class MyWebServer {

  enum page_t {ROOT, BASECONFIG, MODBUSCONFIG};
  
  public:
    MyWebServer();

    void      loop();

  private:
    
    bool      DoReboot;

    #ifdef ESP8266
      using WM_mdns = MDNSResponder;
      using WM_httpUpdater = ESP8266HTTPUpdateServer;
    #elif ESP32
      using WM_mdns = MDNSResponder;
      using WM_httpUpdater = ESPHTTPUpdateServer;
    #endif

    WM_mdns mdns;
    WM_WebServer* server;
    WM_httpUpdater httpUpdater;
        
    void      handleNotFound();
    void      handleReboot();
    void      handleReset();
    void      handleWiFiReset();
    void      handleCSS();
    void      handleJS();
    void      handleJsAjax();
    void      handleJSParam();
    void      handleRoot();
    void      handleBaseConfig();
    void      handleModbusConfig();
    void      handleFavIcon();
    void      handleAjax();
    void      ReceiveJSONConfiguration(page_t page);
    void      getPageHeader(String* html, page_t pageactive);
    void      getPageFooter(String* html);
    
    void      getPage_Status(String* html);
  
};

#endif
