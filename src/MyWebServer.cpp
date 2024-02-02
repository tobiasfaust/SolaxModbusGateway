#include "MyWebServer.h"

MyWebServer::MyWebServer(AsyncWebServer *server, DNSServer* dns): server(server), dns(dns), DoReboot(false), RequestRebootTime(0) {
  
  fsfiles = new handleFiles(server);

  server->begin(); 

  server->onNotFound(std::bind(&MyWebServer::handleNotFound, this, std::placeholders::_1));
  server->on("/", HTTP_GET, std::bind(&MyWebServer::handleRoot, this, std::placeholders::_1));
  server->on("/BaseConfig", HTTP_GET, std::bind(&MyWebServer::handleBaseConfig, this, std::placeholders::_1));
  server->on("/ModbusConfig", HTTP_GET, std::bind(&MyWebServer::handleModbusConfig, this, std::placeholders::_1));
  server->on("/ModbusItemConfig", HTTP_GET, std::bind(&MyWebServer::handleModbusItemConfig, this, std::placeholders::_1));
  server->on("/ModbusRawData", HTTP_GET, std::bind(&MyWebServer::handleModbusRawData, this, std::placeholders::_1));
  server->on("/handleFiles", HTTP_GET, std::bind(&MyWebServer::handleFSFilesWebcontent, this, std::placeholders::_1));

  server->on("^\/(.+)\.(css|js|html|json)$", HTTP_GET, std::bind(&MyWebServer::handleRequestFiles, this, std::placeholders::_1));
  
  server->on("/StoreBaseConfig", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, BASECONFIG));
  server->on("/StoreModbusConfig", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, MODBUSCONFIG));
  server->on("/StoreModbusItemConfig", HTTP_POST, std::bind(&MyWebServer::ReceiveJSONConfiguration, this, std::placeholders::_1, MODBUSITEMCONFIG));
  
  server->on("/favicon.ico", HTTP_GET, std::bind(&MyWebServer::handleFavIcon, this, std::placeholders::_1));
  server->on("/reboot", HTTP_GET, std::bind(&MyWebServer::handleReboot, this, std::placeholders::_1));
  server->on("/reset", HTTP_GET, std::bind(&MyWebServer::handleReset, this, std::placeholders::_1));
  server->on("/wifireset", HTTP_GET, std::bind(&MyWebServer::handleWiFiReset, this, std::placeholders::_1));

  server->on("/ajax", HTTP_POST, std::bind(&MyWebServer::handleAjax, this, std::placeholders::_1));
  server->on("/getitems", HTTP_GET, std::bind(&MyWebServer::handleGetItemJson, this, std::placeholders::_1));
  server->on("/getregister", HTTP_GET, std::bind(&MyWebServer::handleGetRegisterJson, this, std::placeholders::_1));

  server->on("/update", HTTP_GET, std::bind(&MyWebServer::handle_update_page, this, std::placeholders::_1));

  server->on("/update", HTTP_POST, std::bind(&MyWebServer::handle_update_response, this, std::placeholders::_1),
                                   std::bind(&MyWebServer::handle_update_progress, this, std::placeholders::_1, 
                                        std::placeholders::_2,
                                        std::placeholders::_3,
                                        std::placeholders::_4,
                                        std::placeholders::_5,
                                        std::placeholders::_6));

  Serial.println(F("WebServer started..."));
}

void MyWebServer::handle_update_page(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/web/UpdatePage.html", "text/html");
}

void MyWebServer::handle_update_response(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/web/UpdateResponse.html", "text/html");
}

void MyWebServer::handle_update_progress(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
  if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
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
      Serial.printf("Update Success: %uB\n", index+len);
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
      Serial.println("Request to Reboot, wait 5sek ...");
    }
    if (millis() - this->RequestRebootTime > 5000) { // wait 3sek until reboot
      Serial.println("Rebooting...");
      ESP.restart();
    }
  }
}

String MyWebServer::GetReleaseName() {
  return Release;
}

void MyWebServer::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  
  this->getPageHeader(response, ROOT);
  this->getPage_Status(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleRequestFiles(AsyncWebServerRequest *request) {
  File f = LittleFS.open("/" + request->pathArg(0) + "." + request->pathArg(1));
  
  if (!f) {
    if (Config->GetDebugLevel() >=0) {Serial.printf("failed to open requested file: %s.%s", request->pathArg(0), request->pathArg(1));}
    request->send(404, "text/plain", "404: Not found"); 
    return;
  }

  f.close();

  if (request->pathArg(1) == "css") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/css");
  } else if (request->pathArg(1) == "js") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/javascript");
  } else if (request->pathArg(1) == "html") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/html");
  } else if (request->pathArg(1) == "json") {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/json");
  } else {
    request->send(LittleFS, "/" + request->pathArg(0) + "." + request->pathArg(1), "text/plain");
  }

}

void MyWebServer::handleFavIcon(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", FAVICON, sizeof(FAVICON));
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void MyWebServer::handleReboot(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/web/UpdateResponse.html", "text/html"); 

  this->DoReboot = true;
}

void MyWebServer::handleReset(AsyncWebServerRequest *request) {
  if (Config->GetDebugLevel() >= 3) { Serial.println("deletion of all config files was requested ...."); }
  //LittleFS.format(); // Werkszustand -> nur die config dateien loeschen, die register dateien muessen erhalten bleiben
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while(file){
    String path("/"); path.concat(file.name());
    if (path.indexOf(".json") == -1) {Serial.println("Continue"); file = root.openNextFile(); continue;}
    file.close();
    bool f = LittleFS.remove(path);
    if (Config->GetDebugLevel() >= 3) {
      Serial.printf("deletion of configuration file '%s' %s\n", file.name(), (f?"was successful":"has failed"));;
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


void MyWebServer::handleBaseConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, BASECONFIG);
  Config->GetWebContent(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleModbusConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  
  this->getPageHeader(response, MODBUSCONFIG);
  mb->GetWebContentConfig(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleModbusItemConfig(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  
  response->addHeader("Server","ESP Async Web Server");
  
  this->getPageHeader(response, MODBUSITEMCONFIG);
  mb->GetWebContentItemConfig(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleModbusRawData(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, MODBUSRAWDATA);
  mb->GetWebContentRawData(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleFSFilesWebcontent(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server","ESP Async Web Server");

  this->getPageHeader(response, FSFILES);
  fsfiles->GetWebContentConfig(response);
  this->getPageFooter(response);
  request->send(response);
}

void MyWebServer::handleGetItemJson(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  response->addHeader("Pragma", "no-cache");
  response->addHeader("Expires", "-1");
  
  mb->GetLiveDataAsJson(response);

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


void MyWebServer::ReceiveJSONConfiguration(AsyncWebServerRequest *request, page_t page) {
  String json = "{}";

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  String targetPage = "/";
  Serial.print(F("json empfangen: "));
  Serial.println(FPSTR(json.c_str()));  
  
  if (page==BASECONFIG)        { Config->StoreJsonConfig(&json); targetPage = "/BaseConfig"; }
  if (page==MODBUSCONFIG)      { mb->StoreJsonConfig(&json);     targetPage = "/ModbusConfig"; }
  if (page==MODBUSITEMCONFIG)  { mb->StoreJsonItemConfig(&json); targetPage = "/ModbusItemConfig"; }
  
  request->redirect(targetPage);
}

void MyWebServer::handleAjax(AsyncWebServerRequest *request) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  String ret;
  bool RaiseError = false;
  String action, item, newState; 
  String json = "{}";

  AsyncResponseStream *response = request->beginResponseStream("text/json");
  response->addHeader("Server","ESP Async Web Server");

  if(request->hasArg("json")) {
    json = request->arg("json");
  }

  StaticJsonDocument<512> jsonGet; // TODO Use computed size??
  DeserializationError error = deserializeJson(jsonGet, json.c_str());

  StaticJsonDocument<256> jsonReturn;

  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Empfangen: "); }
  if (!error) {
    if (Config->GetDebugLevel() >=4) { serializeJsonPretty(jsonGet, Serial); Serial.println(); }

    if (jsonGet.containsKey("action"))   {action    = jsonGet["action"].as<String>();}
    if (jsonGet.containsKey("item"))     {item      = jsonGet["item"].as<String>();}
    if (jsonGet.containsKey("newState")) {newState  = jsonGet["newState"].as<String>();}
  
  } else { RaiseError = true; }

  if (RaiseError) {
    jsonReturn["error"] = buffer;
    serializeJson(jsonReturn, ret);
    response->print(ret);

    if (Config->GetDebugLevel() >=2) {
      snprintf(buffer, sizeof(buffer), "Ajax Json Command not parseable: %s -> %s", json.c_str(), error.c_str());
      Serial.println(FPSTR(buffer));
    }
    
  } else if (action && action == "RefreshLiveData") {
      mb->GetLiveDataAsJson(response);
  
  } else if (action && action == "SetActiveStatus") {
      if (strcmp(newState.c_str(),"true")==0)  mb->SetItemActiveStatus(item, true); 
      if (strcmp(newState.c_str(),"false")==0) mb->SetItemActiveStatus(item, false);    
      jsonReturn["error"] = "false";
      serializeJson(jsonReturn, ret);
      response->print(ret);

  } else if(action && action == "handlefiles") {
    fsfiles->HandleAjaxRequest(jsonGet, response);

  } else {
    if (Config->GetDebugLevel() >=1) {
      snprintf(buffer, sizeof(buffer), "Ajax Command unknown: %s", action);
      Serial.println(buffer);
    }
    serializeJson(jsonReturn, ret);
    response->print(ret);
  }

  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Antwort: "); Serial.println(ret); }
  
  
  request->send(response);
}

void MyWebServer::getPageHeader(AsyncResponseStream *response, page_t pageactive) {
  response->print("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n");
  response->print("<meta charset='utf-8'>\n");
  response->print("<link rel='stylesheet' type='text/css' href='/web/Style.css'>\n");
  response->print("<script language='javascript' type='text/javascript' src='/web/Javascript.js'></script>\n");
  response->print("<script language='javascript' type='text/javascript' src='/web/JsAjax.js'></script>\n");
  response->print("<title>Solar Inverter Modbus MQTT Gateway</title></head>\n");
  response->print("<body>\n");
  response->print("<table>\n");
  response->print("  <tr>\n");
  response->print("   <td colspan='4'>\n");
  response->print("     <h2>Configuration</h2>\n");
  response->print("   </td>\n");

  response->print("   <td colspan='4' style='color:#CCCCCC;'>\n");
  response->printf("   <i>(%s)</i>\n", Config->GetMqttRoot().c_str());
  response->print("   </td>\n");

  response->print("   <td colspan='7'>\n");
  response->printf("     <b>Release: </b><span style='color:orange;'>%s</span><br>of %s %s", this->GetReleaseName().c_str(), __DATE__, __TIME__);
  response->print("   </td>\n");
  response->print(" </tr>\n");

  response->print(" <tr>\n");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/ModbusConfig'>Modbus Config</a></td>\n", (pageactive==MODBUSCONFIG)?"navi_active":"");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/ModbusItemConfig'>Item Config</a></td>\n", (pageactive==MODBUSITEMCONFIG)?"navi_active":"");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/ModbusRawData'>Raw Data</a></td>\n", (pageactive==MODBUSRAWDATA)?"navi_active":"");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->printf("   <td class='navi %s' style='width: 100px'><a href='/handleFiles'>Files</a></td>\n", (pageactive==FSFILES)?"navi_active":"");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->print("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/SolaxModbusGateway/wiki' target='_blank'>Wiki</a></td>\n");
  response->print("   <td class='navi' style='width: 50px'></td>\n");
  response->print(" </tr>\n");
  response->print("  <tr>\n");
  response->print("   <td colspan='15'>\n");
  response->print("   <p />\n");
}

void MyWebServer::getPageFooter(AsyncResponseStream *response) {
  response->print("</table>\n");
  response->print("<div id='ErrorText' class='errortext'></div>\n");
  response->print("</body>\n");
  response->print("</html>\n");
}

void MyWebServer::getPage_Status(AsyncResponseStream *response) {
  uptime::calculateUptime();
  
  response->print("<table class='editorDemoTable'>\n");
  response->print("<thead>\n");
  response->print("<tr>\n");
  response->print("<td style='width: 250px;'>Name</td>\n");
  response->print("<td style='width: 200px;'>Wert</td>\n");
  response->print("</tr>\n");
  response->print("</thead>\n");
  response->print("<tbody>\n");

  response->print("<tr>\n");
  response->print("<td>IP-Adresse:</td>\n");
  response->printf("<td>%s</td>\n", WiFi.localIP().toString().c_str());
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>WiFi Name:</td>\n");
  response->printf("<td>%s</td>\n", WiFi.SSID().c_str());
  response->print("</tr>\n");
  
  response->print("<tr>\n");
  response->print("<td>MAC:</td>\n");
  response->printf("<td>%s</td>\n", WiFi.macAddress().c_str());
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>WiFi RSSI:</td>\n");
  response->printf("<td>%d</td>\n", WiFi.RSSI());
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>MQTT Status:</td>\n");
  response->printf("<td>%s</td>\n", (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected"));
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Selected Inverter:</td>\n");
  response->printf("<td>%s</td>\n", mb->GetInverterType().c_str());
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Inverter SerialNumber:</td>\n");
  response->printf("<td>%s</td>\n", mb->GetInverterSN().c_str());
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Uptime:</td>\n");
  response->printf("<td>%lu Days, %lu Hours, %lu Minutes</td>\n", uptime::getDays(), uptime::getHours(), uptime::getMinutes()); //uptime_formatter::getUptime().c_str()); //UpTime->getFormatUptime());
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("<td>Free Heap Memory:</td>\n");
  response->printf("<td>%d</td>\n", ESP.getFreeHeap()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("  <td>Firmware Update</td>\n");
  response->print("  <td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n");
  response->print("</tr>\n");
  
  response->print("<tr>\n");
  response->print("  <td>Device Reboot</td>\n");
  response->print("  <td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n");
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("  <td>Werkszustand herstellen (ohne WiFi)</td>\n");
  response->print("  <td><form action='reset'><input class='button' type='submit' value='Reset' /></form></td>\n");
  response->print("</tr>\n");

  response->print("<tr>\n");
  response->print("  <td>WiFi Zugangsdaten entfernen</td>\n");
  response->print("  <td><form action='wifireset'><input class='button' type='submit' value='WifiReset' /></form></td>\n");
  response->print("</tr>\n");
  
  response->print("</tbody>\n");
  response->print("</table>\n"); 

  response->println("<p></p>");
  response->println("<table id='statustable' class='editorDemoTable'>");
  response->println("<thead>");
  response->println("  <tr>");
  response->println("    <td style='width: 250px;'>Name</td>");
  response->println("    <td style='width: 200px;'>LiveData</td>");
  response->println("  </tr>");
  response->println("</thead>");
  response->println("<tbody>");

  response->println("<template id='NewRow'>");
  response->println("  <tr>");
  response->println("    <td>{realname}</td>");
  response->println("    <td><div id='{name}'>{value}</div></td>");
  response->println("  </tr>");
  response->println("</template");

  response->println("</tbody>");
  response->println("</table>");

  response->println("<script language='javascript' type='text/javascript'>");
  response->println("  var url = '/getitems'");
  response->println("    fetch(url)");
  response->println("    .then(response => response.json())");
  response->println("    .then(json => FillItemConfig('#statustable', '#NewRow', 1, json.data));");
  response->println("</script>");
}
