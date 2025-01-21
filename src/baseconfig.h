/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#ifndef BASECONFIG_H_
#define BASECONFIG_H_

#include <commonlibs.h>
#include <ArduinoJson.h>
#include <_Release.h>


class BaseConfig {
 public:
    BaseConfig();
    void      LoadJsonConfig();
    void      GetInitData(JsonDocument& json);

    /**
    * @brief Wrapper function for logging like Serial.printf
    * @param format the format string
    * @param ... the arguments
    */
    void log(const int loglevel, const char* format, ...);
    void log(const int loglevel, const JsonDocument& json);

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
    const uint8_t&  GetSerialRx()      const {return serial_rx;}
    const uint8_t&  GetSerialTx()      const {return serial_tx;}
    const bool&     GetUseAuth()        const { return useAuth; }
    const String&   GetAuthUser()      const {return auth_user;}
    const String&   GetAuthPass()      const {return auth_pass;}

    const String    GetReleaseName();

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
    uint8_t   serial_rx;
    uint8_t   serial_tx;
    bool      useAuth;
    String    auth_user;
    String    auth_pass;
};

extern BaseConfig* Config;

#endif  // BASECONFIG_H_
