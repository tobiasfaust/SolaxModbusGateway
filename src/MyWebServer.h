// https://github.com/esp8266/Arduino/issues/3205
// https://github.com/Hieromon/PageBuilder
// https://www.mediaevent.de/tutorial/sonderzeichen.html
//
// https://byte-style.de/2018/01/automatische-updates-fuer-microcontroller-mit-gitlab-und-platformio/
// https://community.blynk.cc/t/self-updating-from-web-server-http-ota-firmware-for-esp8266-and-esp32/18544
// https://forum.fhem.de/index.php?topic=50628.0

#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include "commonlibs.h"
#include <ArduinoJson.h>
#include "uptime.h" // https://github.com/YiannisBourkelis/Uptime-Library/

#include "baseconfig.h"
#include "modbus.h"
#include "handleFiles.h"
#include "mqtt.h"
#include "favicon.h"
#include "html_update.h"
#include "_Release.h"

class MyWebServer {

  enum page_t {ROOT, BASECONFIG, MODBUSCONFIG, MODBUSITEMCONFIG, MODBUSRAWDATA, FSFILES};
  
  public:
    MyWebServer(AsyncWebServer *server, DNSServer* dns);

    void      loop();

  private:
    
    bool      DoReboot;
    unsigned long RequestRebootTime;

    AsyncWebServer* server;
    DNSServer* dns;

    handleFiles* fsfiles;

    void      handle_update_page(AsyncWebServerRequest *request);
    void      handle_update_progress(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);    
    void      handle_update_response(AsyncWebServerRequest *request);
    void      handleNotFound(AsyncWebServerRequest *request);
    void      handleReboot(AsyncWebServerRequest *request);
    void      handleReset(AsyncWebServerRequest *request);
    void      handleWiFiReset(AsyncWebServerRequest *request);
    void      handleRequestFiles(AsyncWebServerRequest *request);
    void      handleRoot(AsyncWebServerRequest *request);
    void      handleBaseConfig(AsyncWebServerRequest *request);
    void      handleModbusConfig(AsyncWebServerRequest *request);
    void      handleModbusItemConfig(AsyncWebServerRequest *request);
    void      handleModbusRawData(AsyncWebServerRequest *request);
    void      handleFSFilesWebcontent(AsyncWebServerRequest *request);
    void      handleFavIcon(AsyncWebServerRequest *request);
    void      handleAjax(AsyncWebServerRequest *request);
    void      handleGetItemJson(AsyncWebServerRequest *request);
    void      handleGetRegisterJson(AsyncWebServerRequest *request);
    void      ReceiveJSONConfiguration(AsyncWebServerRequest *request, page_t page);
    void      getPageHeader(AsyncResponseStream *response, page_t pageactive);
    void      getPageFooter(AsyncResponseStream *response);
    
    void      getPage_Status(AsyncResponseStream *response);
    String    GetReleaseName();
    
};

#endif
