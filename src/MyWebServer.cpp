#include "MyWebServer.h"

MyWebServer::MyWebServer(AsyncWebServer *server, DNSServer* dns): DoReboot(false), RequestRebootTime(0), server(server), dns(dns) {
  
  fsfiles = new handleFiles(server);

  server->begin(); 

  server->onNotFound(std::bind(&MyWebServer::handleNotFound, this, std::placeholders::_1));
  server->on("/",                       HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1));
   
  server->on("/favicon.ico",            HTTP_GET, std::bind(&MyWebServer::handleFavIcon, this, std::placeholders::_1));
  server->on("/reboot",                 HTTP_GET, std::bind(&MyWebServer::handleReboot, this, std::placeholders::_1));
  server->on("/reset",                  HTTP_GET, std::bind(&MyWebServer::handleReset, this, std::placeholders::_1));
  server->on("/wifireset",              HTTP_GET, std::bind(&MyWebServer::handleWiFiReset, this, std::placeholders::_1));

  server->on("/ajax",                   HTTP_POST, std::bind(&MyWebServer::handleAjax, this, std::placeholders::_1));
  server->on("/getitems",               HTTP_GET, std::bind(&MyWebServer::handleGetItemJson, this, std::placeholders::_1));
  server->on("/getregister",            HTTP_GET, std::bind(&MyWebServer::handleGetRegisterJson, this, std::placeholders::_1));

  server->on("/update",                 HTTP_GET, std::bind(&MyWebServer::handle_update_page, this, std::placeholders::_1));

  server->on("/update",                 HTTP_POST, std::bind(&MyWebServer::handle_update_response, this, std::placeholders::_1),
                                                   std::bind(&MyWebServer::handle_update_progress, this, std::placeholders::_1, 
                                                          std::placeholders::_2,
                                                          std::placeholders::_3,
                                                          std::placeholders::_4,
                                                          std::placeholders::_5,
                                                          std::placeholders::_6));

  if (Config->GetUseAuth()) {
    server->serveStatic("/", LittleFS, "/", "max-age=3600")
          .setDefaultFile("/web/index.html")
          .setAuthentication(Config->GetAuthUser().c_str(), Config->GetAuthPass().c_str());
  } else {
    server->serveStatic("/", LittleFS, "/", "max-age=3600")
          .setDefaultFile("/web/index.html");
  }
  
  dbg.println(F("WebServer started..."));
}

void MyWebServer::handle_update_page(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", HTML_UPDATEPAGE);
  response->addHeader("Server","ESP Async Web Server");
  request->send(response); 
}

void MyWebServer::handle_update_response(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/web/reboot.html", "text/html");
}

void MyWebServer::handle_update_progress(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
  if(!index){
      dbg.printf("Update Start: %s\n", filename.c_str());
      //Update.runAsync(true);
      
      if (filename == "filesystem") {
        if(!Update.begin(LittleFS.totalBytes(), U_SPIFFS)) {
          Update.printError(Serial);
        }
      } else {
        if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
          Update.printError(Serial);
        }
      }
  }
  if(!Update.hasError()){
    if(Update.write(data, len) != len){
        Update.printError(Serial);
    }
  }
  if(final){
    if(Update.end(true)){
      dbg.printf("Update Success: %uB\n", index+len);
      this->DoReboot = true;//Set flag so main loop can issue restart call
    } else {
      Update.printError(Serial);
    }
  }
}

void MyWebServer::loop() {
  //delay(1); // slow response Issue: https://github.com/espressif/arduino-esp32/issues/4348#issuecomment-695115885
  if (this->DoReboot) {
    if (this->RequestRebootTime == 0) {
      this->RequestRebootTime = millis();
      dbg.println("Request to Reboot, wait 5sek ...");
    }
    if (millis() - this->RequestRebootTime > 5000) { // wait 3sek until reboot
      dbg.println("Rebooting...");
      ESP.restart();
    }
  }
}

void MyWebServer::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
  request->redirect("/web/index.html");
}

void MyWebServer::handleFavIcon(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", FAVICON, sizeof(FAVICON));
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void MyWebServer::handleReboot(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/web/reboot.html", "text/html");
  this->DoReboot = true;
}

void MyWebServer::handleReset(AsyncWebServerRequest *request) {
  if (Config->GetDebugLevel() >= 3) { dbg.println("deletion of all config files was requested ...."); }
  //LittleFS.format(); // Werkszustand -> nur die config dateien loeschen, die register dateien muessen erhalten bleiben
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while(file){
    String path("/"); path.concat(file.name());
    if (path.indexOf(".json") == -1) {dbg.println("Continue"); file = root.openNextFile(); continue;}
    file.close();
    bool f = LittleFS.remove(path);
    if (Config->GetDebugLevel() >= 3) {
      dbg.printf("deletion of configuration file '%s' %s\n", file.name(), (f?"was successful":"has failed"));;
    }
    file = root.openNextFile();
  }
  root.close();

  this->handleReboot(request);
}

void MyWebServer::handleWiFiReset(AsyncWebServerRequest *request) {
  #ifdef ESP32
    WiFi.disconnect(true,true);
  #elif ESP8266  
    ESP.eraseConfig();
  #endif
  
  this->handleReboot(request);
}

void MyWebServer::handleGetItemJson(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  
  mb->GetLiveDataAsJson(response, "");

  request->send(response);
}

void MyWebServer::handleGetRegisterJson(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  
  mb->GetRegisterAsJson(response);

  request->send(response);  
}

void MyWebServer::handleAjax(AsyncWebServerRequest *request) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  String ret;
  bool RaiseError = false;
  String action, subaction, item, newState; 
  String json = "{}";

  AsyncResponseStream *response = request->beginResponseStream("text/json");
  response->addHeader("Server","ESP Async Web Server");

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  JsonDocument jsonGet; // TODO Use computed size??
  DeserializationError error = deserializeJson(jsonGet, json.c_str());

  JsonDocument jsonReturn;
  jsonReturn["response"].to<JsonObject>();

  if (Config->GetDebugLevel() >=4) { dbg.print("Ajax Json Empfangen: "); }
  if (!error) {
    if (Config->GetDebugLevel() >=4) { serializeJsonPretty(jsonGet, dbg); dbg.println(); }

    if (jsonGet["action"])   {action    = jsonGet["action"].as<String>();}
    if (jsonGet["subaction"]){subaction = jsonGet["subaction"].as<String>();}
    if (jsonGet["item"])     {item      = jsonGet["item"].as<String>();}
    if (jsonGet["newState"]) {newState  = jsonGet["newState"].as<String>();}
  
  } else { 
    snprintf(buffer, sizeof(buffer), "Ajax Json Command not parseable: %s -> %s", json.c_str(), error.c_str());
    RaiseError = true; 
  }

  if (RaiseError) {
    jsonReturn["response"]["status"] = 0;
    jsonReturn["response"]["text"] = buffer;
    serializeJson(jsonReturn, ret);
    response->print(ret);

    if (Config->GetDebugLevel() >=2) {
      dbg.println(FPSTR(buffer));
    }

    return;

  } else if(action && action == "GetInitData")  {
    if (subaction && subaction == "status") {
      this->GetInitDataStatus(response);
    } else if (subaction && subaction == "navi") {
      this->GetInitDataNavi(response);
    } else if (subaction && subaction == "baseconfig") {
      Config->GetInitData(response);
    } else if (subaction && subaction == "modbusconfig") {
      mb->GetInitData(response);
    } else if (subaction && subaction == "rawdata") {
      mb->GetInitRawData(response);
    }
  
  } else if(action && action == "ReloadConfig")  {
    if (subaction && subaction == "baseconfig") {
      Config->LoadJsonConfig();
    } else if (subaction && subaction == "modbusconfig") {
      mb->LoadJsonConfig(false);
    } else if (subaction && subaction == "modbusitemconfig") {
      mb->LoadJsonItemConfig();
    }
  
    jsonReturn["response"]["status"] = 1;
    jsonReturn["response"]["text"] = "new config reloaded sucessfully";
    serializeJson(jsonReturn, ret);
    response->print(ret);
  
  } else if (action && action == "RefreshLiveData") {
      mb->GetLiveDataAsJson(response, subaction);
  
  } else if (action && action == "SetActiveStatus") {
      if (strcmp(newState.c_str(),"true")==0)  mb->SetItemActiveStatus(item, true); 
      if (strcmp(newState.c_str(),"false")==0) mb->SetItemActiveStatus(item, false);    
      
      jsonReturn["response"]["status"] = 1;
      jsonReturn["response"]["text"] = "successful";
      serializeJson(jsonReturn, ret);
      response->print(ret);

  } else if(action && action == "handlefiles") {
    fsfiles->HandleAjaxRequest(jsonGet, response);

  } else {
    snprintf(buffer, sizeof(buffer), "Ajax Command unknown: %s - %s", action.c_str(), subaction.c_str());
    jsonReturn["response"]["status"] = 0;
    jsonReturn["response"]["text"] = buffer;
    serializeJson(jsonReturn, ret);
    response->print(ret);

    if (Config->GetDebugLevel() >=1) {
      dbg.println(buffer);
    }
  }

  if (Config->GetDebugLevel() >=4) { dbg.print("Ajax Json Antwort: "); dbg.println(ret); }
  
  request->send(response);
}

void MyWebServer::GetInitDataNavi(AsyncResponseStream *response){
  String ret;
  JsonDocument json;
  json["data"].to<JsonObject>();
  json["data"]["hostname"] = Config->GetMqttRoot();
  json["data"]["releasename"] = Config->GetReleaseName();
  json["data"]["releasedate"] = __DATE__;
  json["data"]["releasetime"] = __TIME__;

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);
  response->print(ret);
}

void MyWebServer::GetInitDataStatus(AsyncResponseStream *response) {
  String ret;
  JsonDocument json;
  String rssi = (String)(Config->GetUseETH()?ETH.linkSpeed():WiFi.RSSI());
  if (Config->GetUseETH()) rssi.concat(" Mbps");

  json["data"].to<JsonObject>();
  json["data"]["ipaddress"] = mqtt->GetIPAddress().toString();
  json["data"]["wifiname"] = (Config->GetUseETH()?"wired LAN":WiFi.SSID());
  json["data"]["macaddress"] = WiFi.macAddress();
  json["data"]["rssi"] = rssi;
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

  serializeJson(json, ret);
  response->print(ret);
}