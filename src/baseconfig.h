#ifndef BASECONFIG_H
#define BASECONFIG_H

#include "commonlibs.h"
#include "ArduinoJson.h"


class BaseConfig {

  public:
    BaseConfig();
    void      LoadJsonConfig();
    void      GetInitData(AsyncResponseStream *response);

    const String&   GetMqttServer()    const {return mqtt_server;}
    const uint16_t& GetMqttPort()     const {return mqtt_port;}
    const String&   GetMqttUsername()  const {return mqtt_username;}
    const String&   GetMqttPassword()  const {return mqtt_password;}
    const String&   GetMqttRoot()      const {return mqtt_root;}
    const String&   GetMqttBasePath()  const {return mqtt_basepath;}
    const bool&     UseRandomMQTTClientID() const { return mqtt_UseRandomClientID; }
    const bool&     GetUseETH()        const { return useETH; }
    const String&   GetLANBoard()      const {return LANBoard;}
    const uint8_t&  GetDebugLevel()    const {return debuglevel;}
     
  private:
    String    mqtt_server;
    String    mqtt_username;
    String    mqtt_password;
    uint16_t  mqtt_port;
    String    mqtt_root;
    String    mqtt_basepath;
    bool      mqtt_UseRandomClientID;
    bool      useETH;  // otherwise use WIFI
    String    LANBoard;
    uint8_t   debuglevel;

};

extern BaseConfig* Config;

#endif
