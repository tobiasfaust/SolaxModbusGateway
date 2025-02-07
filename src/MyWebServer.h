/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#ifndef MYWEBSERVER_H_
#define MYWEBSERVER_H_

#include <commonlibs.h>
#include <ArduinoJson.h>
#include <uptime.h> // https://github.com/YiannisBourkelis/Uptime-Library/
#include <uptime_formatter.h>

#include <baseconfig.h>
#include <modbus.h>
#include <handleFiles.h>
#include <mqtt.h>
#include <favicon.h>
#include <ElegantOTA.h>
#include <_Release.h>

class MyWebServer {

  typedef struct {
    uint32_t id;
    String   json;
  } WsConnClient_t;

 public:
    MyWebServer(AsyncWebServer *server, DNSServer* dns);

    void      loop();
    void      sendWebSocketMessage(String& message);

 private:
    
    bool      DoReboot;
    uint64_t  RequestRebootTime;

    std::vector<WsConnClient_t>  WsConnectedClientsForBroadcast = {};
    
    AsyncWebServer* server;
    DNSServer* dns;
    AsyncWebSocket* ws;

    handleFiles* fsfiles;

    void      handleNotFound(AsyncWebServerRequest *request);
    bool      handleReset();
    void      handleRoot(AsyncWebServerRequest *request);
    void      handleFavIcon(AsyncWebServerRequest *request);
//    void      handleGetItemJson(AsyncWebServerRequest *request);
    void      handleGetRegisterJson(AsyncWebServerRequest *request);
//    void      handleGetSetterJson(AsyncWebServerRequest *request);
//    void      GetInitDataStatus(AsyncResponseStream *response);
//    void      GetInitDataNavi(AsyncResponseStream *response);
    void      GetInitDataStatus(JsonDocument& json);
    void      GetInitDataNavi(JsonDocument& json);    

    void      onImprovWiFiConnectedCb(const char *ssid, const char *password);
    void      onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
};

#endif  // MYWEBSERVER_H_
