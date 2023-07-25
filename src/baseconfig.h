#ifndef BASECONFIG_H
#define BASECONFIG_H

#include "commonlibs.h"
#include "ArduinoJson.h"

#define _CONFIG_JSON_DOC_SIZE 1024

class BaseConfig {

  public:
    BaseConfig();
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(AsyncResponseStream *response);

    const String&   GetMqttServer()    const {return mqtt_server;}
    const uint16_t& GetMqttPort()     const {return mqtt_port;}
    const String&   GetMqttUsername()  const {return mqtt_username;}
    const String&   GetMqttPassword()  const {return mqtt_password;}
    const String&   GetMqttRoot()      const {return mqtt_root;}
    const String&   GetMqttBasePath()  const {return mqtt_basepath;}
    const bool&     UseRandomMQTTClientID() const { return mqtt_UseRandomClientID; }
    const uint8_t&  GetDebugLevel()    const {return debuglevel;}
    const String&   GetWebPostURL()    const {return webPost_URL;}
    const String&   GetWebPostUser()    const {return webPost_User;}
    const String&   GetWebPostPass()    const {return webPost_Pass;}
    const uint16_t& GetWebPostFrequency()    const {return webPost_Frequency;}
     
  private:
    String    mqtt_server;
    String    mqtt_username;
    String    mqtt_password;
    uint16_t  mqtt_port;
    String    mqtt_root;
    String    mqtt_basepath;
    bool      mqtt_UseRandomClientID;
    uint8_t   debuglevel;
    String    webPost_URL;
    String    webPost_User;
    String    webPost_Pass;
    uint16_t  webPost_Frequency;

};

extern BaseConfig* Config;

#endif
