#include "baseconfig.h"

BaseConfig::BaseConfig() : debuglevel(0) {  
  #ifdef ESP8266
    SPIFFS.begin();
  #elif ESP32
    SPIFFS.begin(true); // true: format SPIFFS/NVS if mount fails
  #endif
  
  // Flash Write Issue
  // https://github.com/esp8266/Arduino/issues/4061#issuecomment-428007580
  //SPIFFS.format();
  
  LoadJsonConfig();
}

void BaseConfig::StoreJsonConfig(String* json) {

  StaticJsonDocument<512> doc;
  deserializeJson(doc, *json);
  JsonObject root = doc.as<JsonObject>();

  if (!root.isNull()) {
    File configFile = SPIFFS.open("/BaseConfig.json", "w");
    if (!configFile) {
      if (this->GetDebugLevel() >=0) {Serial.println("failed to open BaseConfig.json file for writing");}
    } else {  
      serializeJsonPretty(doc, Serial);
      if (serializeJson(doc, configFile) == 0) {
        if (this->GetDebugLevel() >=0) {Serial.println(F("Failed to write to file"));}
      }
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void BaseConfig::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (SPIFFS.exists("/BaseConfig.json")) {
    //file exists, reading and loading
    Serial.println("reading config file");
    File configFile = SPIFFS.open("/BaseConfig.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      //size_t size = configFile.size();

      StaticJsonDocument<512> doc; // TODO Use computed size??
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error) {
        serializeJsonPretty(doc, Serial);
        
        if (doc.containsKey("mqttroot"))         { this->mqtt_root = doc["mqttroot"].as<String>();} else {this->mqtt_root = "solax";}
        if (doc.containsKey("mqttserver"))       { this->mqtt_server = doc["mqttserver"].as<String>();} else {this->mqtt_server = "test.mosquitto.org";}
        if (doc.containsKey("mqttport"))         { this->mqtt_port = (int)(doc["mqttport"]);} else {this->mqtt_port = 1883;}
        if (doc.containsKey("mqttuser"))         { this->mqtt_username = doc["mqttuser"].as<String>();} else {this->mqtt_username = "";}
        if (doc.containsKey("mqttpass"))         { this->mqtt_password = doc["mqttpass"].as<String>();} else {this->mqtt_password = "";}
        if (doc.containsKey("mqttbasepath"))     { this->mqtt_basepath = doc["mqttbasepath"].as<String>();} else {this->mqtt_basepath = "home/";}
        if (doc.containsKey("UseRandomClientID")){ if (strcmp(doc["UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}} else {this->mqtt_UseRandomClientID = true;}
        if (doc.containsKey("debuglevel"))       { this->debuglevel = _max((int)(doc["debuglevel"]), 0);} else {this->debuglevel = 0; }
      } else {
        if (this->GetDebugLevel() >=1) {Serial.println("failed to load json config, load default config");}
        loadDefaultConfig = true;
      }
    }
  } else {
    if (this->GetDebugLevel() >=3) {Serial.println("BaseConfig.json config File not exists, load default config");}
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    this->mqtt_server = "test.mosquitto.org";
    this->mqtt_port  = 1883;
    this->mqtt_username = "";
    this->mqtt_password = "";
    this->mqtt_root = "Solax";
    this->mqtt_basepath = "home/";
    this->mqtt_UseRandomClientID = true;
    this->debuglevel = 0;
    
    loadDefaultConfig = false; //set back
  }

  // Data Cleaning
  if(this->mqtt_basepath.endsWith("/")) {
    this->mqtt_basepath = this->mqtt_basepath.substring(0, this->mqtt_basepath.length()-1); 
  }

}

void BaseConfig::GetWebContent(WM_WebServer* server) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  String html = "";

  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 250px;'>Name</td>\n");
  html.concat("<td style='width: 200px;'>Wert</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("<tr>\n");
  html.concat("<td>Device Name</td>\n");
  sprintf(buffer, "<td><input size='30' maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", this->mqtt_root.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");
  
  html.concat("<tr>\n");
  html.concat("<td>MQTT Server IP</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttserver' type='text' value='%s'/></td>\n", this->mqtt_server.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");
  
  html.concat("<tr>\n");
  html.concat("<td>MQTT Server Port</td>\n");
  sprintf(buffer, "<td><input maxlength='5' name='mqttport' type='text' style='width: 6em' value='%d'/></td>\n", this->mqtt_port);
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>MQTT Authentification: Username (optional)</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttuser' type='text' value='%s'/></td>\n", this->mqtt_username.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>MQTT Authentification: Password (optional)</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttpass' type='text' value='%s'/></td>\n", this->mqtt_password.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>MQTT Topic Base Path (example: home/inverter)</td>\n");
  sprintf(buffer, "<td><input size='30' maxlength='40' name='mqttbasepath' type='text' value='%s'/></td>\n", this->mqtt_basepath.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("<tr>\n");
  html.concat("  <td colspan='2'>\n");
  
  html.concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_URCID1' name='UseRandomClientID' value='none' %s />", (this->mqtt_UseRandomClientID)?"":"checked");
  html.concat(buffer);
  html.concat("<label for='sel_URCID1'>benutze statische MQTT ClientID</label></div>\n");
  
  html.concat("    <div class='inline'>");
  sprintf(buffer, "<input type='radio' id='sel_URCID2' name='UseRandomClientID' value='yes' %s />", (this->mqtt_UseRandomClientID)?"checked":"");
  html.concat(buffer);
  html.concat("<label for='sel_URCID2'>benutze dynamische MQTT ClientID</label></div>\n");
    
  html.concat("  </td>\n");
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>DebugMode (0 [off] .. 5 [max]</td>\n");
  sprintf(buffer, "<td><input min='0' max='5' name='debuglevel' type='number' style='width: 6em' value='%d'/></td>\n", this->debuglevel);
  html.concat(buffer);
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("</tbody>\n");
  html.concat("</table>\n");


  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreBaseConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");

  server->sendContent(html.c_str()); html = "";
}
