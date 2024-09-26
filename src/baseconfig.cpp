#include "baseconfig.h"

BaseConfig::BaseConfig() : debuglevel(0), serial_rx(3), serial_tx(1) {  
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
      
      if (!error && doc["data"]) {
        serializeJsonPretty(doc, dbg);
        
        if (doc["data"]["mqttroot"])         { this->mqtt_root = doc["data"]["mqttroot"].as<String>();} else {this->mqtt_root = "solax";}
        if (doc["data"]["mqttserver"])       { this->mqtt_server = doc["data"]["mqttserver"].as<String>();} else {this->mqtt_server = "test.mosquitto.org";}
        if (doc["data"]["mqttport"])         { this->mqtt_port = (int)(doc["data"]["mqttport"]);} else {this->mqtt_port = 1883;}
        if (doc["data"]["mqttuser"])         { this->mqtt_username = doc["data"]["mqttuser"].as<String>();} else {this->mqtt_username = "";}
        if (doc["data"]["mqttpass"])         { this->mqtt_password = doc["data"]["mqttpass"].as<String>();} else {this->mqtt_password = "";}
        if (doc["data"]["mqttbasepath"])     { this->mqtt_basepath = doc["data"]["mqttbasepath"].as<String>();} else {this->mqtt_basepath = "home/";}
        if (doc["data"]["UseRandomClientID"]){ if (strcmp(doc["data"]["UseRandomClientID"], "none")==0) { this->mqtt_UseRandomClientID=false;} else {this->mqtt_UseRandomClientID=true;}} else {this->mqtt_UseRandomClientID = true;}
        if (doc["data"]["SelectConnectivity"]){if (strcmp(doc["data"]["SelectConnectivity"], "wifi")==0) { this->useETH=false;} else {this->useETH=true;}} else {this->useETH = false;}
        if (doc["data"]["debuglevel"])       { this->debuglevel = _max((int)(doc["data"]["debuglevel"]), 0);} else {this->debuglevel = 0; }
        if (doc["data"]["SelectLAN"])        {this->LANBoard = doc["data"]["SelectLAN"].as<String>();} else {this->LANBoard = "";}
        if (doc["data"]["serial_rx"])        { this->serial_rx = (doc["serial_rx"].as<int>());}
        if (doc["data"]["serial_tx"])        { this->serial_tx = (doc["serial_tx"].as<int>());}
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

  #ifdef USE_WEBSERIAL
    json["data"]["tr_serial_rx"]["className"] = "hide";
    json["data"]["tr_serial_tx"]["className"] = "hide";
  #else
    json["data"]["GpioPin_serial_rx"] = this->serial_rx;
    json["data"]["GpioPin_serial_tx"] = this->serial_tx;
  #endif

  json["response"].to<JsonObject>();
  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
  serializeJson(json, ret);
  response->print(ret);
}