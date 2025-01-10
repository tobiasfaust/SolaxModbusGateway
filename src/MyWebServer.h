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
#include "uptime_formatter.h"

#include "baseconfig.h"
#include "modbus.h"
#include "handleFiles.h"
#include "mqtt.h"
#include "favicon.h"
#include <ElegantOTA.h>
#include "_Release.h"

class MyWebServer {

  public:
    MyWebServer(AsyncWebServer *server, DNSServer* dns);

    void      loop();
    void      sendWebSocketMessage(const String& message);

  private:
    
    bool      DoReboot;
    unsigned long RequestRebootTime;

    std::vector<uint32_t>  WsConnectedClientsForBroadcast = {};
    
    AsyncWebServer* server;
    DNSServer* dns;
    AsyncWebSocket* ws;

    handleFiles* fsfiles;

    void      handleNotFound(AsyncWebServerRequest *request);
    void      handleReboot(AsyncWebServerRequest *request);
    void      handleReset(AsyncWebServerRequest *request);
    void      handleWiFiReset(AsyncWebServerRequest *request);
    void      handleRoot(AsyncWebServerRequest *request);
    void      handleFavIcon(AsyncWebServerRequest *request);
    void      handleAjax(AsyncWebServerRequest *request);
    void      handleGetItemJson(AsyncWebServerRequest *request);
    void      handleGetRegisterJson(AsyncWebServerRequest *request);

    void      GetInitDataStatus(AsyncResponseStream *response);
    void      GetInitDataNavi(AsyncResponseStream *response);
    void      GetInitDataStatus(JsonDocument& json);
    void      GetInitDataNavi(JsonDocument& json);    

    void      onImprovWiFiConnectedCb(const char *ssid, const char *password);
    void      onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
};

#endif
