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

  struct wsclient_t {
    uint32_t ws_id;
    enum requestData_t {LOG_DATA, MODBUS_DATA} requestData;
    // Equality operator needed for push_back_unique / std::find
    bool operator==(const wsclient_t& other) const {
      return ws_id == other.ws_id && requestData == other.requestData;
    }
  };

 public:
    MyWebServer(fs::LittleFSFS& sysFS, fs::LittleFSFS& configFS, AsyncWebServer *server, DNSServer* dns);

    void      loop();
    void      sendWebSocketMessage(String& message, String JsonRequest, uint32_t wsclient_id);
    void      logGetValuesCallback(const char* logline, JsonDocument& json, uint32_t wsclient_id);

 private:
    
    fs::LittleFSFS&   sysFS;
    fs::LittleFSFS&   configFS;
    bool              DoReboot;
    uint64_t          RequestRebootTime;
      
      
    AsyncWebServer* server;
    DNSServer* dns;
    AsyncWebSocket* ws;
    handleFiles* fsfiles;

    std::vector<wsclient_t>* _wsclientRequests = nullptr;

    void      handleNotFound(AsyncWebServerRequest *request);
    bool      handleReset();
    void      handleRoot(AsyncWebServerRequest *request);
    void      handleFavIcon(AsyncWebServerRequest *request);
    void      GetInitDataStatus(JsonDocument& json);
    void      GetInitDataNavi(JsonDocument& json);    

    void      onImprovWiFiConnectedCb(const char *ssid, const char *password);
    void      onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
};

#endif  // MYWEBSERVER_H_

