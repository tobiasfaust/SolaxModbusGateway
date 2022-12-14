#include "MyWebServer.h"

MyWebServer::MyWebServer() : DoReboot(false) {
  server = new WM_WebServer(80);

  if (!MDNS.begin(Config->GetMqttRoot().c_str()))  {  
    Serial.println(F("Error setting up MDNS responder!"));  
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
  else {  
    Serial.println(F("mDNS responder started"));  
  }


  //httpUpdater = new WM_httpUpdater(false);
  httpUpdater.setup(server);
  
  server->begin(); 

  server->onNotFound([this]() { this->handleNotFound(); });
  server->on("/", [this]() {this->handleRoot(); });
  server->on("/BaseConfig", [this]() {this->handleBaseConfig(); });
  server->on("/ModbusConfig", [this]() {this->handleModbusConfig(); });
  server->on("/ModbusItemConfig", [this]() {this->handleModbusItemConfig(); });
  server->on("/ModbusRawData", [this]() {this->handleModbusRawData(); });
  

  server->on("/style.css", HTTP_GET, [this]() {this->handleCSS(); });
  server->on("/javascript.js", HTTP_GET, [this]() {this->handleJS(); });
  server->on("/jsajax.js", HTTP_GET, [this]() {this->handleJsAjax(); });
  
  server->on("/StoreBaseConfig", HTTP_POST, [this]()   { this->ReceiveJSONConfiguration(BASECONFIG); });
  server->on("/StoreModbusConfig", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(MODBUSCONFIG); });
  server->on("/StoreModbusItemConfig", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(MODBUSITEMCONFIG); });
  
  server->on("/favicon.ico", HTTP_GET, [this]()        { this->handleFavIcon(); });
  server->on("/reboot", HTTP_GET, [this]()             { this->handleReboot(); });
  server->on("/reset", HTTP_GET, [this]()              { this->handleReset(); });
  server->on("/wifireset", HTTP_GET, [this]()          { this->handleWiFiReset(); });

  
  server->on("/ajax", [this]() {this->handleAjax(); });
  
  Serial.println(F("WebServer started..."));
}

void MyWebServer::loop() {
  server->handleClient();
  delay(1); // slow response Issue: https://github.com/espressif/arduino-esp32/issues/4348#issuecomment-695115885
  if (this->DoReboot) {ESP.restart();}
}

void MyWebServer::handleNotFound() {
  server->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot() {
  String html;
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  this->getPageHeader(&html, ROOT);
  this->getPage_Status(&html);
  html.concat("<p></p>");
  server->sendContent(html.c_str()); html = "";
  mb->GetWebContentActiveLiveData(server);
  this->getPageFooter(&html);
  server->sendContent(html.c_str()); 
  server->sendContent("");
}

void MyWebServer::handleCSS() {
  server->send_P(200, "text/css", STYLE_CSS);
}

void MyWebServer::handleFavIcon() {
  server->send_P(200, "image/x-icon", FAVICON, sizeof(FAVICON));
}

void MyWebServer::handleJS() {

  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/javascript", "");
  
  server->sendContent(JAVASCRIPT); 
  
  server->sendContent("");  
}

void MyWebServer::handleJsAjax() {
  server->send_P(200, "text/javascript", JSAJAX);
}

void MyWebServer::handleReboot() {
  server->sendHeader("Location","/");
  server->send(303); 
  this->DoReboot = true;  
}

void MyWebServer::handleReset() {
  SPIFFS.format();
  this->handleReboot();
}

void MyWebServer::handleWiFiReset() {
  #ifdef ESP32
    WiFi.disconnect(true,true);
  #elif ESP8266  
    ESP.eraseConfig();
  #endif
  
  this->handleReboot();
}


void MyWebServer::handleBaseConfig() {
  String html;
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  this->getPageHeader(&html, BASECONFIG);
  server->sendContent(html.c_str()); html = "";
  Config->GetWebContent(server);
  this->getPageFooter(&html);
  server->sendContent(html.c_str()); 
  server->sendContent("");
}

void MyWebServer::handleModbusConfig() {
  String html;
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  this->getPageHeader(&html, MODBUSCONFIG);
  server->sendContent(html.c_str()); html = "";
  mb->GetWebContentConfig(server);
  this->getPageFooter(&html);
  server->sendContent(html.c_str()); 
  server->sendContent("");
}

void MyWebServer::handleModbusItemConfig() {
  String html;
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  this->getPageHeader(&html, MODBUSITEMCONFIG);
  server->sendContent(html.c_str()); html = "";
  mb->GetWebContentItemConfig(server);
  this->getPageFooter(&html);
  server->sendContent(html.c_str()); 
  server->sendContent("");
}

void MyWebServer::handleModbusRawData() {
  String html;
  server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server->sendHeader("Pragma", "no-cache");
  server->sendHeader("Expires", "-1");
  server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  server->send(200, "text/html", "");
  this->getPageHeader(&html, MODBUSRAWDATA);
  server->sendContent(html.c_str()); html = "";
  mb->GetWebContentRawData(server);
  this->getPageFooter(&html);
  server->sendContent(html.c_str()); 
  server->sendContent("");
}

void MyWebServer::ReceiveJSONConfiguration(page_t page) {
  String json = server->arg("json");
  String targetPage = "/";
  Serial.print(F("json empfangen: "));
  Serial.println(FPSTR(json.c_str()));  
  
  if (page==BASECONFIG)        { Config->StoreJsonConfig(&json); targetPage = "/BaseConfig"; }
  if (page==MODBUSCONFIG)      { mb->StoreJsonConfig(&json);     targetPage = "/ModbusConfig"; }
  if (page==MODBUSITEMCONFIG)  { mb->StoreJsonItemConfig(&json); targetPage = "/ModbusItemConfig"; }
  
  server->sendHeader("Location", targetPage.c_str());
  server->send(303); 
}

void MyWebServer::handleAjax() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  String ret;
  bool RaiseError = false;
  String action, item, newState; 
  
  StaticJsonDocument<512> jsonGet; // TODO Use computed size??
  DeserializationError error = deserializeJson(jsonGet, server->arg("json"));

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

    if (Config->GetDebugLevel() >=2) {
      snprintf(buffer, sizeof(buffer), "Ajax Json Command not parseable: %s -> %s", server->arg("json").c_str(), error.c_str());
      Serial.println(FPSTR(buffer));
    }
    
  } else if (action && action == "RefreshLiveData") {
      ret = mb->GetLiveDataAsJson();
  
  } else if (action && action == "SetActiveStatus") {
      if (strcmp(newState.c_str(),"true")==0)  mb->SetItemActiveStatus(item, true); 
      if (strcmp(newState.c_str(),"false")==0) mb->SetItemActiveStatus(item, false);    
      jsonReturn["error"] = "false";
      serializeJson(jsonReturn, ret);
  
  } else {
    if (Config->GetDebugLevel() >=1) {
      snprintf(buffer, sizeof(buffer), "Ajax Command unknown: %s", action);
      Serial.println(buffer);
    }
    serializeJson(jsonReturn, ret);
  }

  if (Config->GetDebugLevel() >=4) { Serial.print("Ajax Json Antwort: "); Serial.println(ret); }
  server->send(200, "text/html", ret.c_str());
}

void MyWebServer::getPageHeader(String* html, page_t pageactive) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  html->concat("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n");
  html->concat("<meta charset='utf-8'>\n");
  html->concat("<link rel='stylesheet' type='text/css' href='/style.css'>\n");
  //html->concat("<script language='javascript' type='text/javascript' src='/parameter.js'></script>\n");
  html->concat("<script language='javascript' type='text/javascript' src='/javascript.js'></script>\n");
  html->concat("<script language='javascript' type='text/javascript' src='/jsajax.js'></script>\n");
  html->concat("<title>Solax X1/X3 G4 Modbus MQTT Gateway</title></head>\n");
  html->concat("<body>\n");
  html->concat("<table>\n");
  html->concat("  <tr>\n");
  html->concat("   <td colspan='4'>\n");
  html->concat("     <h2>Configuration</h2>\n");
  html->concat("   </td>\n");

  html->concat("   <td colspan='4' style='color:#CCCCCC;'>\n");
  sprintf(buffer, "   <i>(%s)</i>\n", Config->GetMqttRoot().c_str());
  html->concat(buffer);
  html->concat("   </td>\n");

  html->concat("   <td colspan='5'>\n");
  sprintf(buffer, "     <b>Release: </b><span style='color:orange;'>%s</span><br>of %s %s", "none", __DATE__, __TIME__);
  html->concat(buffer);
  html->concat("   </td>\n");
  html->concat(" </tr>\n");

  html->concat(" <tr>\n");
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/ModbusConfig'>Modbus Config</a></td>\n", (pageactive==MODBUSCONFIG)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/ModbusItemConfig'>Item Config</a></td>\n", (pageactive==MODBUSITEMCONFIG)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/ModbusRawData'>Raw Data</a></td>\n", (pageactive==MODBUSRAWDATA)?"navi_active":"");
  html->concat(buffer);
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  html->concat("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/SolaxModbusGateway/wiki' target='_blank'>Wiki</a></td>\n");
  html->concat("   <td class='navi' style='width: 50px'></td>\n");
  html->concat(" </tr>\n");
  html->concat("  <tr>\n");
  html->concat("   <td colspan='13'>\n");
  html->concat("   <p />\n");
}

void MyWebServer::getPageFooter(String* html) {
  html->concat("</table>\n");
  html->concat("<div id='ErrorText' class='errortext'></div>\n");
  html->concat("</body>\n");
  html->concat("</html>\n");
}

void MyWebServer::getPage_Status(String* html) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  uptime::calculateUptime();
  
  html->concat("<table class='editorDemoTable'>\n");
  html->concat("<thead>\n");
  html->concat("<tr>\n");
  html->concat("<td style='width: 250px;'>Name</td>\n");
  html->concat("<td style='width: 200px;'>Wert</td>\n");
  html->concat("</tr>\n");
  html->concat("</thead>\n");
  html->concat("<tbody>\n");

  html->concat("<tr>\n");
  html->concat("<td>IP-Adresse:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.localIP().toString().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>WiFi Name:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.SSID().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");
  
  html->concat("<tr>\n");
  html->concat("<td>MAC:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.macAddress().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>WiFi RSSI:</td>\n");
  sprintf(buffer, "<td>%d</td>\n", WiFi.RSSI());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>MQTT Status:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", (mqtt->GetConnectStatusMqtt()?"Connected":"Not Connected"));
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Selected Inverter:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", mb->GetInverterType().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Inverter SerialNumber:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", mb->GetInverterSN().c_str());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Uptime:</td>\n");
  sprintf(buffer, "<td>%lu Days, %lu Hours, %lu Minutes</td>\n", uptime::getDays(), uptime::getHours(), uptime::getMinutes()); //uptime_formatter::getUptime().c_str()); //UpTime->getFormatUptime());
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("<td>Free Heap Memory:</td>\n");
  sprintf(buffer, "<td>%d</td>\n", ESP.getFreeHeap()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  html->concat(buffer);
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("  <td>Firmware Update</td>\n");
  html->concat("  <td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n");
  html->concat("</tr>\n");
  
  html->concat("<tr>\n");
  html->concat("  <td>Device Reboot</td>\n");
  html->concat("  <td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n");
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("  <td>Werkszustand herstellen (ohne WiFi)</td>\n");
  html->concat("  <td><form action='reset'><input class='button' type='submit' value='Reset' /></form></td>\n");
  html->concat("</tr>\n");

  html->concat("<tr>\n");
  html->concat("  <td>WiFi Zugangsdaten entfernen</td>\n");
  html->concat("  <td><form action='wifireset'><input class='button' type='submit' value='WifiReset' /></form></td>\n");
  html->concat("</tr>\n");
  
  html->concat("</tbody>\n");
  html->concat("</table>\n");     
}
