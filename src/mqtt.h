/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#ifndef MQTT_H_
#define MQTT_H_

#include <commonlibs.h>
#include <PubSubClient.h>
#include <ImprovWiFiLibrary.h>
#include <baseconfig.h>

#include <vector>
#include <cstdio>
#include <utility>

#ifdef ESP8266
  // #define SetHostName(x) wifi_station_set_hostname(x);
  #define ESP_getChipId() ESP.getChipId()
#endif

#ifdef ESP32
  #include <ETH.h>
  // #define SetHostName(x) WiFi.getHostname(x); --> MQTT.cpp TODO
  #define ESP_getChipId() static_cast<uint32_t>(ESP.getEfuseMac())   // Unterschied zu ESP.getFlashChipId() ???
#endif


#ifdef ESP32

  struct eth_shield_t {
  String name;
  uint8_t PHY_ADDR;
  int PHY_POWER;
  int PHY_MDC;
  int PHY_MDIO;
  eth_phy_type_t  PHY_TYPE;
  eth_clock_mode_t CLK_MODE;
  std::vector<uint8_t> blockedGpio; // gpios blocked by eth device, DO not use them for other tasks!

  eth_shield_t(const String& n,
               uint8_t addr,
               int power,
               int mdc,
               int mdio,
               eth_phy_type_t phy,
               eth_clock_mode_t clk,
               std::initializer_list<uint8_t> blk = {})
    : name(n),
      PHY_ADDR(addr),
      PHY_POWER(power),
      PHY_MDC(mdc),
      PHY_MDIO(mdio),
      PHY_TYPE(phy),
      CLK_MODE(clk),
      blockedGpio(blk) {}
};

#elif defined(ESP8266)
  typedef struct {
      String name;
  } eth_shield_t;
#endif

class MQTT: PubSubClient {
  #ifdef ESP32
    std::vector<eth_shield_t> lan_shields = {
        {"WT32-ETH01", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, {16, 23, 18, 19, 21, 22}}
        //{"TTGO T-ETH", 0, 0, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, {2, 4, 15}},
        //{"AI-Thinker", 0, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, {2, 4, 15}},
        //{"M5Stack",   0, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, {2, 4, 15}}
        };
  #elif defined(ESP8266)
    std::vector<eth_shield_t> lan_shields = {
      {"test1"},
      {"test2"}};
  #endif

 public:
    MQTT(const char* MqttServer,
          uint16_t MqttPort,
          String MqttBasepath,
          String MqttRoot);
    void              loop();
    void              Publish_Bool(const char* subtopic, bool b, bool fulltopic);
    void              Publish_Int(const char* subtopic, int number, bool fulltopic);
    void              Publish_Float(const char* subtopic, float number, bool fulltopic);
    void              Publish_String(const char* subtopic, String value, bool fulltopic);
    void              Publish_IP();
    String            getTopic(String subtopic, bool fulltopic);
    void              disconnect();
    const String&     GetRoot()  const {return mqtt_root;}
    const String&     GetBasePath() const {return mqtt_basepath;}
    void              Subscribe(String topic);
    bool              UnSubscribe(String topic);
    void              ClearSubscriptions();

    const bool&       GetConnectStatusWifi()      const {return ConnectStatusWifi;}
    const bool&       GetConnectStatusMqtt()      const {return ConnectStatusMqtt;}
    const IPAddress&  GetIPAddress()              const {return ipadresse;}

    using PubSubClient::setCallback;

    ImprovWiFi        improvSerial;

 protected:
    void              reconnect();

 private:
    WiFiClient        espClient;

    std::vector<String>* subscriptions = NULL;

    String            mqtt_root = "";
    String            mqtt_basepath = "";
    uint64_t          mqttreconnect_lasttry = 0;
    uint64_t          last_debugmsg = 0;
    uint64_t          last_keepalivemsg = 0;
    bool              ConnectStatusWifi;
    bool              ConnectStatusMqtt;
    IPAddress         ipadresse;

  #ifdef ESP32
    void              WifiOnEvent(WiFiEvent_t event);
  #endif

    void              WaitForConnect();
    void              onImprovWiFiErrorCb(ImprovTypes::Error err);
    eth_shield_t*     GetEthShield(String ShieldName);
};

extern MQTT* mqtt;

#endif  // MQTT_H_
