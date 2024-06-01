#ifndef MQTT_H
#define MQTT_H

#include "commonlibs.h" 
#include <PubSubClient.h>
#include <ESPAsyncWiFiManager.h>    // https://github.com/alanswx/ESPAsyncWiFiManager
#include <vector>
#include "baseconfig.h"
#include <ETH.h>

#ifdef ESP8266
  //#define SetHostName(x) wifi_station_set_hostname(x);
  #define ESP_getChipId() ESP.getChipId() 
#elif ESP32
  //#define SetHostName(x) WiFi.getHostname(x); --> MQTT.cpp TODO
  #define ESP_getChipId() (uint32_t)ESP.getEfuseMac()   // Unterschied zu ESP.getFlashChipId() ???
#endif

#if defined(ESP8266) || defined(ESP32)
  #include <functional>
  #define CALLBACK_FUNCTION std::function<void(char*, uint8_t*, unsigned int)> MyCallback
#else
  #define CALLBACK_FUNCTION void (*MyCallback)(char*, uint8_t*, unsigned int)
#endif

typedef struct {
    String name; 
    uint8_t PHY_ADDR;
    int PHY_POWER; 
    int PHY_MDC;
    int PHY_MDIO; 
    eth_phy_type_t  PHY_TYPE;
    eth_clock_mode_t CLK_MODE;
} eth_shield_t;

class MQTT {

  std::vector<eth_shield_t> lan_shields = {{"WT32-ETH01", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN}, 
                                            {"test", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN}};

  public:

    MQTT(AsyncWebServer* server, DNSServer *dns, const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot, char* APName, char* APpassword);
    void              loop();
    void              Publish_Bool(const char* subtopic, bool b, bool fulltopic);
    void              Publish_Int(const char* subtopic, int number, bool fulltopic);
    void              Publish_Float(const char* subtopic, float number, bool fulltopic);
    void              Publish_String(const char* subtopic, String value, bool fulltopic);
    void              Publish_IP();
    String            getTopic(String subtopic, bool fulltopic);
    void              setCallback(CALLBACK_FUNCTION);
    void              disconnect();
    const String&     GetRoot()  const {return mqtt_root;};
    void              Subscribe(String topic);
    void              ClearSubscriptions();
    
    const bool&       GetConnectStatusWifi()      const {return ConnectStatusWifi;}
    const bool&       GetConnectStatusMqtt()      const {return ConnectStatusMqtt;}
    const IPAddress&  GetIPAddress()              const {return ipadresse;}

  protected:
    PubSubClient*     mqtt;
    void              reconnect();

  private:
    uint8_t           pin_Relais1;          // Pin zum Abfragen des ersten Potetialfreien Kontakts
    uint8_t           pin_Relais2;          // Pin zum Abfragen des ersten Potetialfreien Kontakts
    uint8_t           default_pin_Relais1;          // Pin zum Abfragen des ersten Potetialfreien Kontakts
    uint8_t           default_pin_Relais2;          // Pin zum Abfragen des ersten Potetialfreien Kontakts

    AsyncWebServer*   server;
    DNSServer*        dns;
    WiFiClient        espClient;
    AsyncWiFiManager* wifiManager;

    CALLBACK_FUNCTION;
    void              callback(char* topic, byte* payload, unsigned int length);
    
    std::vector<String>* subscriptions = NULL;

    String            mqtt_root = "";
    String            mqtt_basepath = "";
    unsigned long     mqttreconnect_lasttry = 0;
    unsigned long     last_keepalive = 0;
    bool              ConnectStatusWifi;
    bool              ConnectStatusMqtt;
    IPAddress         ipadresse;
  
    void              WifiOnEvent(WiFiEvent_t event);
    void              WaitForConnect();
    eth_shield_t*     GetEthShield(String ShieldName);

};

extern MQTT* mqtt;

#endif
