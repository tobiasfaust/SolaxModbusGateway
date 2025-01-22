/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#include "MyWebServer.h"

MyWebServer::MyWebServer(AsyncWebServer *server, DNSServer* dns): 
        DoReboot(false),
        RequestRebootTime(0),
        server(server),
        dns(dns) {
  
  fsfiles = new handleFiles(server);
  ws = new AsyncWebSocket("/ajaxws");

  server->onNotFound(std::bind(&MyWebServer::handleNotFound, this, std::placeholders::_1));
  server->on("/",                       HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1));
   
  server->on("/favicon.ico",            HTTP_GET, std::bind(&MyWebServer::handleFavIcon, this, std::placeholders::_1));
  server->on("/getitems",               HTTP_GET, [&](AsyncWebServerRequest *request){ mb->GetLiveDataAsJsonToWebServer(request); });
  //server->on("/getregister",            HTTP_GET, std::bind(&MyWebServer::handleGetRegisterJson, this, std::placeholders::_1)); // deprecated, not longer in use
  server->on("/getsetter",              HTTP_GET, [&](AsyncWebServerRequest *request){ mb->GetSettersAsJsonToWebServer(request); });


  ws->onEvent(std::bind(&MyWebServer::onWsEvent, this, std::placeholders::_1, 
                                                       std::placeholders::_2, 
                                                       std::placeholders::_3, 
                                                       std::placeholders::_4, 
                                                       std::placeholders::_5, 
                                                       std::placeholders::_6 ));

  server->addHandler(ws);
  
  ElegantOTA.begin(server);    // Start ElegantOTA
  ElegantOTA.setGitEnv(String(GIT_OWNER), String(GIT_REPO), String(GIT_BRANCH));
  ElegantOTA.setFWVersion(String(Config->GetReleaseName() + " / Build: " + GITHUB_RUN ));
  ElegantOTA.setBackupRestoreFS("/config");
  ElegantOTA.setAutoReboot(true);
  
  //ElegantOTA callbacks
  //ElegantOTA.onStart(onOTAStart);
  //ElegantOTA.onProgress(onOTAProgress);
  //ElegantOTA.onEnd(std::bind(&MyWebServer::onOTAEnd, this, std::placeholders::_1));

  if (Config->GetUseAuth()) {
    server->serveStatic("/", LittleFS, "/", "max-age=3600")
          .setDefaultFile("/web/index.html")
          .setAuthentication(Config->GetAuthUser().c_str(), Config->GetAuthPass().c_str());
  } else {
    server->serveStatic("/", LittleFS, "/", "max-age=3600")
          .setDefaultFile("/web/index.html");
  }
  
  // try to start the server if wifi is connected, otherwise wait for wifi connection
  if (mqtt->GetConnectStatusWifi()) {
    server->begin();
    Config->log(1, "WebServer has been started ...");
  } else {
    mqtt->improvSerial.onImprovConnected(std::bind(&MyWebServer::onImprovWiFiConnectedCb, this, std::placeholders::_1, std::placeholders::_2));
  }
}

void MyWebServer::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Config->log(2, "[Client: %u] WebSocket client connected", client->id());
  
  } else if (type == WS_EVT_DISCONNECT) {
    Config->log(2, "[Client: %u] WebSocket client disconnected", client->id());

    // wenn client->id() in der Liste WsConnectedClientsForBroadcast vorhanden ist, dann entfernen
    
    auto it = std::find_if(WsConnectedClientsForBroadcast.begin(), WsConnectedClientsForBroadcast.end(), 
                 [client](const WsConnClient_t& c) { return c.id == client->id(); });
    if (it != WsConnectedClientsForBroadcast.end()) {
      WsConnectedClientsForBroadcast.erase(it);
    }

    // wenn keine clients mehr in der Liste sind, dann den Callback für die modbuswerte entfernen
    if (this->WsConnectedClientsForBroadcast.size() == 0) {
      mb->setWebSocketCallback(nullptr);
    }
  
  } else if (type == WS_EVT_DATA) {
    String msg(""); msg.reserve(len + 1);
    for (size_t i = 0; i < len; i++) { msg += (char)data[i]; } msg += '\0';
    Config->log(2, "[Client: %u] WebSocket data received: %s", client->id(), msg.c_str()); 

    // message json request format: {"cmd": {"action": "GetInitData", "subaction": "status"}} // subaction optional
    // message json response format: die Antwort wird im json ergänzt, so weiß der Requestor zu welchem Command die Antwort gehört: 
    // Example: {"cmd": {"action": "GetInitData", "subaction": "status"}, "response": {"status": 1, "text": "successful"}, "data": {"ipaddress": "", "wifiname": "", "macaddress": "", "rssi": "", "bssid": "", "mqtt_status": "", "inverter_type": "", "inverter_serial": "", "uptime": "", "freeheapmem": ""}}
    // Ausnahme: kontinuierliches Streaming der modbuswerte, hier wird kein response und nicht das ursprüngliche Command zurückgegeben
    // example: {"data-id":{ "registername": "value", "registername": "value", ...}}

    String action(""), subaction(""), item("");
    bool newState = false;
    JsonDocument json;
    DeserializationError error = deserializeJson(json, msg.c_str());
    if (!error) {
      if (json["cmd"]) {
        if (json["cmd"]["action"])   {action    = json["cmd"]["action"].as<String>();}
        if (json["cmd"]["subaction"]){subaction = json["cmd"]["subaction"].as<String>();}
        if (json["cmd"]["item"])     {item      = json["cmd"]["item"].as<String>();}
        if (json["cmd"]["newState"]) {newState  = (json["cmd"]["newState"].as<String>() == "true"?true:false);}
        
      }

      if (action == "GetItemsAsStream") {
        // add client id to the list of clients to broadcast if not already in the list
        auto it = std::find_if(WsConnectedClientsForBroadcast.begin(), WsConnectedClientsForBroadcast.end(), 
           [client](const WsConnClient_t& c) { return c.id == client->id(); });
        if (it == WsConnectedClientsForBroadcast.end()) {
          const WsConnClient_t w = {client->id(), msg};
          WsConnectedClientsForBroadcast.push_back(w);
        }
        // if this is the first client in the list, then set the callback for the modbus values
        if (this->WsConnectedClientsForBroadcast.size() == 1) {
          mb->setWebSocketCallback([this](String& message) {
            this->sendWebSocketMessage(message);
          });
        }
        return;
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
        } else if (subaction && subaction == "modbusconfig") {
          mb->GetInitData(json);
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
      Config->log(1, "WebSocket data received but not a valid json string: %s -> %s", msg.c_str(), error.c_str());
      json["response"]["status"] = 0;
      json["response"]["text"] = error.c_str();
    }

    ws->text(client->id(), json.as<String>());

  }
}

void MyWebServer::onImprovWiFiConnectedCb(const char *ssid, const char *password) {
  server->begin();
  Config->log(1, "WebServer has been started now ...");
}

void MyWebServer::sendWebSocketMessage(String& message) {
  // send message to all connected clients in the list WsConnectedClientsForBroadcast
  for (auto client : WsConnectedClientsForBroadcast) {
    if (ws->client(client.id)) {
      message = message.substring(0, message.length()-1) + "," + client.json.substring(1, client.json.length()-1);

      Config->log(4, "send WebSocket Message to client %u: %s", client.id, message.c_str());
      ws->text(client.id, message);
    }
  }
    
}

void MyWebServer::loop() {
  //delay(1); // slow response Issue: https://github.com/espressif/arduino-esp32/issues/4348#issuecomment-695115885
  if (this->DoReboot) {
    if (this->RequestRebootTime == 0) {
      this->RequestRebootTime = millis();
      Config->log(1, "Request to Reboot, wait 5sek ...");
    }
    if (millis() - this->RequestRebootTime > 5000) { // wait 3sek until reboot
      Config->log(1, "Rebooting...");
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
  request->redirect("/web/index.html");
}

void MyWebServer::handleFavIcon(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", FAVICON, sizeof(FAVICON));
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

bool MyWebServer::handleReset() {
  bool ret = true;
  Config->log(3, "deletion of all config files was requested ....");
  //LittleFS.format(); // Werkszustand -> nur die config dateien loeschen, die register dateien muessen erhalten bleiben
  File root = LittleFS.open("/config/");
  File file = root.openNextFile();
  while(file){
    String path("/config/"); path.concat(file.name());
    if (path.indexOf(".json") == -1) {file = root.openNextFile(); continue;}
    file.close();
    
    if (LittleFS.remove(path)) {
      Config->log(4, "deletion of configuration file '%s' was successful", file.name());
    } else {
      Config->log(2, "deletion of configuration file '%s' has failed", file.name());
      ret = false;
    }
    file = root.openNextFile();
  }
  root.close();
  this->DoReboot = true;

  return ret;
}

void MyWebServer::handleGetRegisterJson(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  
  mb->GetRegisterAsJsonToWebServer(response);
  request->send(response);  
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

  #ifndef USE_WEBSERIAL
    json["data"]["tr_webserial"]["className"] = "hide";
  #endif

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
}