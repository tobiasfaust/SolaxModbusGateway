/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#include <baseconfig.h>

BaseConfig::BaseConfig(): debuglevel(2),
                          serial_rx(3),
                          serial_tx(1),
                          mqtt_UseRandomClientID(true),
                          useAuth(false) {
  #ifdef ESP8266
    LittleFS.begin();
  #elif defined(ESP32)
    if (LittleFS.begin(true)) {  // true: format LittleFS/NVS if mount fails
      if (!LittleFS.exists("/config")) {
        LittleFS.mkdir("/config");
      }
    } else {
      this->logN(1, "LittleFS Mount Failed");
    }
  #endif

  // Flash Write Issue
  // https://github.com/esp8266/Arduino/issues/4061#issuecomment-428007580
  // LittleFS.format();

  LoadJsonConfig();
}

void BaseConfig::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (LittleFS.exists("/config/baseconfig.json")) {
    // file exists, reading and loading
    this->logN(2, "reading config file");
    File configFile = LittleFS.open("/config/baseconfig.json", "r");
    if (configFile) {
      this->logN(2, "opened config file");

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, configFile);

      if (!error && doc["data"]) {
        this->log(1, doc);

        if (doc["data"]["mqttroot"])         { this->mqtt_root = doc["data"]["mqttroot"].as<String>();} else {this->mqtt_root = "solax";}
        if (doc["data"]["mqttserver"])       { this->mqtt_server = doc["data"]["mqttserver"].as<String>();} else {this->mqtt_server = "test.mosquitto.org";}
        if (doc["data"]["mqttport"])         { this->mqtt_port = doc["data"]["mqttport"].as<uint16_t>();} else {this->mqtt_port = 1883;}
        if (doc["data"]["mqttuser"])         { this->mqtt_username = doc["data"]["mqttuser"].as<String>();} else {this->mqtt_username = "";}
        if (doc["data"]["mqttpass"])         { this->mqtt_password = doc["data"]["mqttpass"].as<String>();} else {this->mqtt_password = "";}
        if (doc["data"]["mqttbasepath"])     { this->mqtt_basepath = doc["data"]["mqttbasepath"].as<String>();} else {this->mqtt_basepath = "home/";}
        if (doc["data"]["SelectConnectivity"]){if (strcmp(doc["data"]["SelectConnectivity"], "wifi")==0) { this->useETH=false;} else {this->useETH=true;}} else {this->useETH = false;}
        if (doc["data"]["debuglevel"])       { this->debuglevel = _max(doc["data"]["debuglevel"].as<uint8_t>(), 0);} else {this->debuglevel = 0; }
        if (doc["data"]["SelectLAN"])        { this->LANBoard = doc["data"]["SelectLAN"].as<String>();} else {this->LANBoard = "";}
        if (doc["data"]["serial_rx"])        { this->serial_rx = doc["data"]["serial_rx"].as<uint8_t>(); } else {this->serial_rx = 3;}
        if (doc["data"]["serial_tx"])        { this->serial_tx = doc["data"]["serial_tx"].as<uint8_t>(); } else {this->serial_tx = 1;}
        if (doc["data"]["auth_user"])        { this->auth_user = doc["data"]["auth_user"].as<String>();} else {this->auth_user = "admin";}
        if (doc["data"]["auth_pass"])        { this->auth_pass = doc["data"]["auth_pass"].as<String>();} else {this->auth_pass = "password";}
      
        this->useAuth                     = doc["data"]["sel_auth"].as<bool>();
        this->mqtt_UseRandomClientID      = doc["data"]["useRandomClientID"].as<bool>();
      
      } else {
        this->logN(1, "failed to load json config, load default config");
        loadDefaultConfig = true;
      }
    }
  } else {
    this->logN(3, "baseconfig.json config File not exists, load default config");
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
    this->debuglevel = 2;
    this->LANBoard = "";
    
    loadDefaultConfig = false; //set back
  }

  // Data Cleaning
  if(this->mqtt_basepath.endsWith("/")) {
    this->mqtt_basepath = this->mqtt_basepath.substring(0, this->mqtt_basepath.length()-1); 
  }

}

const String BaseConfig::GetReleaseName() {
  return String(Release) + "(@" + String(GIT_BRANCH) + ")"; 
}

void BaseConfig::GetInitData(JsonDocument& json) {
  json["data"]["mqttroot"]    = this->mqtt_root;
  json["data"]["mqttserver"]  = this->mqtt_server;
  json["data"]["mqttport"]    = this->mqtt_port;
  json["data"]["mqttuser"]    = this->mqtt_username;
  json["data"]["mqttpass"]    = this->mqtt_password;
  json["data"]["mqttbasepath"]= this->mqtt_basepath;
  json["data"]["debuglevel"]  = this->debuglevel;
  json["data"]["sel_wifi"]    = ((this->useETH)?0:1);
  json["data"]["sel_eth"]     = ((this->useETH)?1:0);
  json["data"]["useRandomClientID"]  = ((this->mqtt_UseRandomClientID)?1:0);
  json["data"]["sel_auth"]= ((this->useAuth)?1:0);
  json["data"]["auth_user"]   = this->auth_user;
  json["data"]["auth_pass"]   = this->auth_pass;


  #ifdef USE_WEBSERIAL
    json["data"]["tr_serial_rx"]["className"] = "hide";
    json["data"]["tr_serial_tx"]["className"] = "hide";
  #else
    json["data"]["GpioPin_serial_rx"] = this->serial_rx;
    json["data"]["GpioPin_serial_tx"] = this->serial_tx;
  #endif

  json["response"]["status"] = 1;
  json["response"]["text"] = "successful";
}

void BaseConfig::logN(const int loglevel, const char* format, ...) {
  if (this->GetDebugLevel() < loglevel) return;
  
  va_list args;
  va_start(args, format);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  #ifdef USE_WEBSERIAL
    WebSerial.printf("[Log %d] ", loglevel);
    WebSerial.println(buffer);
  #else
    Serial.printf("[Log %d] ", loglevel);
    Serial.println(buffer);
  #endif
  va_end(args);
}

void BaseConfig::log(const int loglevel, const char* format, ...) {
  if (this->GetDebugLevel() < loglevel) return;
  
  va_list args;
  va_start(args, format);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  #ifdef USE_WEBSERIAL
    WebSerial.print(buffer);
  #else
    Serial.print(buffer);
  #endif
  va_end(args);
}

void BaseConfig::log(const int loglevel, const JsonDocument& json) {
  if (this->GetDebugLevel() < loglevel) return;
  
  #ifdef USE_WEBSERIAL
    serializeJsonPretty(json, WebSerial);
    WebSerial.println();
  #else
    serializeJsonPretty(json, Serial);
    Serial.println();
  #endif
}