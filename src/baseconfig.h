#ifndef BASECONFIG_H
#define BASECONFIG_H

#include "commonlibs.h"
#include "ArduinoJson.h"


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
     
  private:
    String    mqtt_server;
    String    mqtt_username;
    String    mqtt_password;
    uint16_t  mqtt_port;
    String    mqtt_root;
    String    mqtt_basepath;
    bool      mqtt_UseRandomClientID;
    uint8_t   debuglevel;

};

extern BaseConfig* Config;

#endif
