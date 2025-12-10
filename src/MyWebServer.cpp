/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#include "MyWebServer.h"

MyWebServer::MyWebServer(fs::LittleFSFS& sysFS, fs::LittleFSFS& configFS, AsyncWebServer *server, DNSServer* dns)
    : sysFS(sysFS), configFS(configFS), DoReboot(false), RequestRebootTime(0), server(server), dns(dns) {
  
  fsfiles = new handleFiles(server);
  fsfiles->registerLogCallback(std::bind(&BaseConfig::logN, Config, std::placeholders::_1, std::placeholders::_2));
  fsfiles->registerLittleFS(&sysFS, "/web");
  fsfiles->registerLittleFS(&configFS, "/config");

  _wsclientRequests = new std::vector<wsclient_t>();

  ws = new AsyncWebSocket("/ajaxws");

  server->onNotFound(std::bind(&MyWebServer::handleNotFound, this, std::placeholders::_1));
  server->on("/",                       HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1), nullptr, nullptr);
   
  server->on("/favicon.ico",            HTTP_GET, std::bind(&MyWebServer::handleFavIcon, this, std::placeholders::_1), nullptr, nullptr);
  server->on("/getitems",               HTTP_GET, [&](AsyncWebServerRequest *request){ mb->GetLiveDataAsJsonToWebServer(request); });
  server->on("/getsetter",              HTTP_GET, [&](AsyncWebServerRequest *request){ mb->GetSettersAsJsonToWebServer(request); });


  ws->onEvent(std::bind(&MyWebServer::onWsEvent, this, std::placeholders::_1, 
                                                       std::placeholders::_2, 
                                                       std::placeholders::_3, 
                                                       std::placeholders::_4, 
                                                       std::placeholders::_5, 
                                                       std::placeholders::_6 ));

  server->addHandler(ws);
  
  ElegantOTA.begin(server);    // Start ElegantOTA
  ElegantOTA.setGitEnv(String(GIT_OWNER), String(GIT_REPO), String(GIT_BRANCH), String(GITHUB_RUN).toInt());
  ElegantOTA.setFWVersion(String(Config->GetReleaseName() + " / Build: " + GITHUB_RUN ));
  ElegantOTA.setTargetPartition("webdata");  // Set default partition for OTA updates
  ElegantOTA.setAutoReboot(true);
  
  //ElegantOTA callbacks
  //ElegantOTA.onStart(onOTAStart);
  //ElegantOTA.onProgress(onOTAProgress);
  //ElegantOTA.onEnd(std::bind(&MyWebServer::onOTAEnd, this, std::placeholders::_1));

  if (Config->GetUseAuth()) {
    server->serveStatic("/web/", sysFS, "/", "max-age=3600")
          .setDefaultFile("/web/web/index.html")
          .setAuthentication(Config->GetAuthUser().c_str(), Config->GetAuthPass().c_str());
  } else {
    server->serveStatic("/web/", sysFS, "/", "max-age=3600")
          .setDefaultFile("/web/web/index.html");
  }

  server->serveStatic("/config/", configFS, "/");
  
  // try to start the server if wifi is connected, otherwise wait for wifi connection
  if (mqtt->GetConnectStatusWifi()) {
    server->begin();
    Config->logN(1, "WebServer has been started ...");
  } else {
    mqtt->improvSerial.onImprovConnected(std::bind(&MyWebServer::onImprovWiFiConnectedCb, this, std::placeholders::_1, std::placeholders::_2));
  }
}

void MyWebServer::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Config->logN(2, "[Client: %u] WebSocket client connected", client->id());
  
  } else if (type == WS_EVT_DISCONNECT) {
    Config->logN(2, "[Client: %u] WebSocket client disconnected", client->id());

    // Remove client from WebSocket client requests if it exists
    for (uint8_t i = 0; i < _wsclientRequests->size(); i++) {
      if (_wsclientRequests->at(i).ws_id == client->id()) {
        
        if (_wsclientRequests->at(i).requestData == wsclient_t::MODBUS_DATA) { mb->onValues(nullptr, nullptr); }
        if (_wsclientRequests->at(i).requestData == wsclient_t::LOG_DATA) { Config->onLogValues(nullptr); }

        _wsclientRequests->erase(_wsclientRequests->begin() + i);
      }
    }
    _wsclientRequests->shrink_to_fit();
  
  } else if (type == WS_EVT_DATA) {
    String msg(""); msg.reserve(len + 1);
    for (size_t i = 0; i < len; i++) { msg += (char)data[i]; } msg += '\0';
    Config->logN(2, "[Client: %u] WebSocket data received: %s", client->id(), msg.c_str()); 

    // message json request format: {"cmd": {"action": "GetInitData", "subaction": "status"}} // subaction optional
    // message json response format: die Antwort wird im json ergänzt, so weiß der Requestor zu welchem Command die Antwort gehört: 
    // Example: {"cmd": {"action": "GetInitData", "subaction": "status"}, "response": {"status": 1, "text": "successful"}, "data": {"ipaddress": "", "wifiname": "", "macaddress": "", "rssi": "", "bssid": "", "mqtt_status": "", "inverter_type": "", "inverter_serial": "", "uptime": "", "freeheapmem": ""}}
    // Ausnahme: kontinuierliches Streaming der modbuswerte, hier wird kein response und nicht das ursprüngliche Command zurückgegeben
    // example: {"data-id":{ "registername": "value", "registername": "value", ...}}

    String action(""), subaction(""), item("");
    std::list<String>* options = nullptr;
    bool newState = false;
    JsonDocument json;
    DeserializationError error = deserializeJson(json, msg.c_str());
    if (!error) {
      // {"cmd":{"action":"subscribe","subaction":"modbus_data","filter":"onlyactive","highlight":"true","opts":"[+unit]"}}
      if (json["cmd"]) {
        if (json["cmd"]["action"])   {action    = json["cmd"]["action"].as<String>();}
        if (json["cmd"]["subaction"]){subaction = json["cmd"]["subaction"].as<String>();}
        if (json["cmd"]["item"])     {item      = json["cmd"]["item"].as<String>();}
        if (json["cmd"]["newState"]) {newState  = (json["cmd"]["newState"].as<String>() == "true"?true:false);}
        
        if (json["cmd"]["opts"]) {
          // prüfe ob optionen übergeben wurden, validiere format [string1, string2, ...] und überführe die opts in -> std::list<String>* options
          Serial.printf("opts: %s\n", json["cmd"]["opts"].as<String>().c_str());
          JsonArray optsArray = json["cmd"]["opts"].as<JsonArray>();
          options = new std::list<String>();
          for (String opt : optsArray) {
            Serial.printf("opt: %s\n", opt.c_str());
            opt.toLowerCase();
            options->push_back(opt);
          }
        }
      }

      if (action && action == "subscribe") {
        if (subaction && subaction == "log_data") {
          push_back_unique(_wsclientRequests, {client->id(), wsclient_t::LOG_DATA});
          Config->onLogValues(std::bind(&MyWebServer::logGetValuesCallback, this, std::placeholders::_1, json, client->id()));
        } else if (subaction && subaction == "modbus_data") {
          push_back_unique(_wsclientRequests, {client->id(), wsclient_t::MODBUS_DATA});
          mb->onValues(std::bind(&MyWebServer::sendWebSocketMessage, this, std::placeholders::_1, msg, client->id()), options);
        }
      }

      if (action && action == "reset") {
        if (handleReset()) {
          json["response"]["status"] = 1;
          json["response"]["text"] = "all config files deleted successfully";
        } else {
          json["response"]["status"] = 0;
          json["response"]["text"] = "deletion of config files failed";
        }
      }

      if(action && action == "reboot") {
        this->DoReboot = true;
        json["response"]["status"] = 1;
        json["response"]["text"] = "reboot after 5sec...";
      }

      if(action && action == "GetInitData")  {
        if (subaction && subaction == "status") {
          this->GetInitDataStatus(json);
        } else if (subaction && subaction == "navi") {
          this->GetInitDataNavi(json);
        } else if (subaction && subaction == "baseconfig") {
          Config->GetInitData(json);
          json["js"]["gpio_disabled"] = Config->disabledGPIO.getArrayExcludeIdentifier(BaseConfig::GpioIdentifier::BASECONFIG);
        } else if (subaction && subaction == "modbusconfig") {
          mb->GetInitData(json);
          json["js"]["gpio_disabled"] = Config->disabledGPIO.getArrayExcludeIdentifier(BaseConfig::GpioIdentifier::MODBUS);
        } else if (subaction && subaction == "rawdata") {
          mb->GetInitRawData(json);
        }
      }

      if(action && action == "ReloadConfig")  {
        if (subaction && subaction == "baseconfig") {
          Config->LoadJsonConfig();
        } else if (subaction && subaction == "modbusconfig") {
          mb->LoadJsonConfig(false);
        } else if (subaction && subaction == "modbusitemconfig") {
          mb->LoadJsonItemConfig();
        }
      
        json["response"]["status"] = 1;
        json["response"]["text"] = "new config reloaded sucessfully";
      }

      if (action && action == "SetActiveStatus") {
        mb->SetItemActiveStatus(item, newState);    
        
        json["response"]["status"] = 1;
        json["response"]["text"] = String("item successfully set to " + String(newState ? "active" : "inactive"));
      } 
      
      if(action && action == "handlefiles") {
        fsfiles->HandleRequest(json);
      }

    } else {
      Config->logN(1, "WebSocket data received but not a valid json string: %s -> %s", msg.c_str(), error.c_str());
      json["response"]["status"] = 0;
      json["response"]["text"] = error.c_str();
    }

    ws->text(client->id(), json.as<String>());

  }
}

void MyWebServer::onImprovWiFiConnectedCb(const char *ssid, const char *password) {
  server->begin();
  Config->logN(1, "WebServer has been started now ...");
}

void MyWebServer::loop() {
  //delay(1); // slow response Issue: https://github.com/espressif/arduino-esp32/issues/4348#issuecomment-695115885
  if (this->DoReboot) {
    if (this->RequestRebootTime == 0) {
      this->RequestRebootTime = millis();
      Config->logN(1, "Request to Reboot, wait 5sek ...");
    }
    if (millis() - this->RequestRebootTime > 5000) { // wait 3sek until reboot
      Config->logN(1, "Rebooting...");
      ESP.restart();
    }
  }
  ElegantOTA.loop();
  ws->cleanupClients();
}

void MyWebServer::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
  request->redirect("/web/web/index.html");
}

void MyWebServer::handleFavIcon(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", FAVICON, sizeof(FAVICON));
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

bool MyWebServer::handleReset() {
  Config->logN(3, "deletion of all config files was requested ....");

  bool result = configFS.format();
  if (!result) {
    Config->logN(2, "formatting of config Filesystem failed");
  } else {
    Config->logN(4, "formatting of config Filesystem was successful");
  }
  this->DoReboot = true;

  return result;
}

void MyWebServer::GetInitDataNavi(JsonDocument& json) {
  json["data"].to<JsonObject>();
  json["data"]["hostname"] = Config->GetMqttRoot();
  json["data"]["releasename"] = Config->GetReleaseName();
  json["data"]["releasedate"] = __DATE__;
  json["data"]["releasetime"] = __TIME__;

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
}

void MyWebServer::GetInitDataStatus(JsonDocument& json) {
  String rssi = (String)(Config->GetUseETH()?ETH.linkSpeed():WiFi.RSSI());
  if (Config->GetUseETH()) rssi.concat(" Mbps");

  json["data"].to<JsonObject>();
  json["data"]["ipaddress"] = mqtt->GetIPAddress().toString();
  json["data"]["wifiname"] = (Config->GetUseETH()?"wired LAN":WiFi.SSID());
  json["data"]["macaddress"] = WiFi.macAddress();
  json["data"]["rssi"] = rssi;
  json["data"]["bssid"] = (Config->GetUseETH()?"wired LAN":WiFi.BSSIDstr());
  json["data"]["mqtt_status"] = (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected");
  json["data"]["inverter_type"] = mb->GetInverterType();
  json["data"]["inverter_serial"] = mb->GetInverterSN();
  json["data"]["uptime"] = uptime_formatter::getUptime();
  json["data"]["freeheapmem"] = ESP.getFreeHeap();

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
}

void MyWebServer::logGetValuesCallback(const char* logline, JsonDocument& json, uint32_t wsclient_id) {
  json["logline"] = logline;
  this->ws->text(wsclient_id, json.as<String>());
}

void MyWebServer::sendWebSocketMessage(String& message, String JsonRequest, uint32_t wsclient_id) {
  // add original Request json to message
  message = message.substring(0, message.length()-1) + "," + JsonRequest.substring(1, JsonRequest.length()-1);
  Config->logN(4, "send WebSocket Message to client %u: %s", wsclient_id, message.c_str());

  this->ws->text(wsclient_id, message);
}