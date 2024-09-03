#include "baseconfig.h"

BaseConfig::BaseConfig() : debuglevel(0) {  
  #ifdef ESP8266
    LittleFS.begin();
  #elif ESP32
    if (!LittleFS.begin(true)) { // true: format LittleFS/NVS if mount fails
      dbg.println("LittleFS Mount Failed");
    }
  #endif
  
  // Flash Write Issue
  // https://github.com/esp8266/Arduino/issues/4061#issuecomment-428007580
  // LittleFS.format();
  
  LoadJsonConfig();
}

void BaseConfig::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (LittleFS.exists("/baseconfig.json")) {
    //file exists, reading and loading
    dbg.println("reading config file");
    File configFile = LittleFS.open("/baseconfig.json", "r");
    if (configFile) {
      dbg.println("opened config file");
      
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, configFile);
      
      if (!error && doc.containsKey("data")) {
        serializeJsonPretty(doc, dbg);
        
        if (doc["data"].containsKey("mqttroot"))         { this->mqtt_root = doc["data"]["mqttroot"].as<String>();} else {this->mqtt_root = "solax";}
        if (doc["data"].containsKey("mqttserver"))       { this->mqtt_server = doc["data"]["mqttserver"].as<String>();} else {this->mqtt_server = "test.mosquitto.org";}
        if (doc["data"].containsKey("mqttport"))         { this->mqtt_port = (int)(doc["data"]["mqttport"]);} else {this->mqtt_port = 1883;}
        if (doc["data"].containsKey("mqttuser"))         { this->mqtt_username = doc["data"]["mqttuser"].as<String>();} else {this->mqtt_username = "";}
        if (doc["data"].containsKey("mqttpass"))         { this->mqtt_password = doc["data"]["mqttpass"].as<String>();} else {this->mqtt_password = "";}
        if (doc["data"].containsKey("mqttbasepath"))     { this->mqtt_basepath = doc["data"]["mqttbasepath"].as<String>();} else {this->mqtt_basepath = "home/";}
        if (doc["data"].containsKey("UseRandomClientID")){ if (strcmp(doc["data"]["UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}} else {this->mqtt_UseRandomClientID = true;}
        if (doc["data"].containsKey("SelectConnectivity")){if (strcmp(doc["data"]["SelectConnectivity"], "wifi")==0) { this->useETH=false;} else {this->useETH=true;}} else {this->useETH = false;}
        if (doc["data"].containsKey("debuglevel"))       { this->debuglevel = _max((int)(doc["data"]["debuglevel"]), 0);} else {this->debuglevel = 0; }
        if (doc["data"].containsKey("SelectLAN"))        {this->LANBoard = doc["data"]["SelectLAN"].as<String>();} else {this->LANBoard = "";}
      } else {
        if (this->GetDebugLevel() >=1) {dbg.println("failed to load json config, load default config");}
        loadDefaultConfig = true;
      }
    }
  } else {
    if (this->GetDebugLevel() >=3) {dbg.println("BaseConfig.json config File not exists, load default config");}
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
    this->useETH = false;
    this->debuglevel = 0;
    this->LANBoard = "";
    
    loadDefaultConfig = false; //set back
  }

  // Data Cleaning
  if(this->mqtt_basepath.endsWith("/")) {
    this->mqtt_basepath = this->mqtt_basepath.substring(0, this->mqtt_basepath.length()-1); 
  }

}

const String BaseConfig::GetReleaseName() {
  return String(Release) + "(@" + GIT_BRANCH + ")"; 
}

void BaseConfig::GetInitData(AsyncResponseStream *response) {
  String ret;
  JsonDocument json;
  json["data"].to<JsonObject>();
  json["data"]["mqttroot"]    = this->mqtt_root;
  json["data"]["mqttserver"]  = this->mqtt_server;
  json["data"]["mqttport"]    = this->mqtt_port;
  json["data"]["mqttuser"]    = this->mqtt_username;
  json["data"]["mqttpass"]    = this->mqtt_password;
  json["data"]["mqttbasepath"]= this->mqtt_basepath;
  json["data"]["debuglevel"]  = this->debuglevel;
  json["data"]["sel_wifi"]    = ((this->useETH)?0:1);
  json["data"]["sel_eth"]     = ((this->useETH)?1:0);
  json["data"]["sel_URCID1"]  = ((this->mqtt_UseRandomClientID)?0:1);
  json["data"]["sel_URCID2"]  = ((this->mqtt_UseRandomClientID)?1:0);

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);
  response->print(ret);
}