#include "mqtt.h"

MQTT::MQTT(AsyncWebServer* server, DNSServer *dns, const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot, char* APName, char* APpassword): 
server(server), dns(dns), mqtt_root(MqttRoot), mqtt_basepath(MqttBasepath), ConnectStatusWifi(false), ConnectStatusMqtt(false) { 
  
  this->subscriptions = new std::vector<String>{};
 
  this->mqtt = new PubSubClient();
  WiFi.setHostname(this->mqtt_root.c_str());
  WiFi.onEvent(std::bind(&MQTT::WifiOnEvent, this, std::placeholders::_1));

  if (Config->GetDebugLevel() >=3) {
    dbg.printf("Go into %s Mode\n", (Config->GetUseETH()?"ETH":"Wifi"));
  }

  if (Config->GetUseETH()) {
    //ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
    eth_shield_t* shield = this->GetEthShield(Config->GetLANBoard());
    ETH.begin(shield->PHY_ADDR,
              shield->PHY_POWER,
              shield->PHY_MDC,
              shield->PHY_MDIO,
              shield->PHY_TYPE,
              shield->CLK_MODE);

    this->WaitForConnect();
  } else {

    WiFi.mode(WIFI_STA);
    this->wifiManager = new AsyncWiFiManager(server, dns);

    if (Config->GetDebugLevel() >=4) wifiManager->setDebugOutput(true); 
      else wifiManager->setDebugOutput(false); 

    wifiManager->setConnectTimeout(60);
    wifiManager->setConfigPortalTimeout(300);
    dbg.println("WiFi Start");
    
  //  wifiManager->startConfigPortal("OnDemandAP");

    if (!wifiManager->autoConnect(APName, APpassword) ) {
      dbg.println("failed to connect and start configPortal");
      wifiManager->startConfigPortal(APName, APpassword);
    }
  }
  
  //WiFi.printDiag(Serial);

  dbg.printf("Starting MQTT (%s:%d)\n", MqttServer, MqttPort);
  espClient = WiFiClient();
  
  this->mqtt->setClient(espClient);
  this->mqtt->setServer(MqttServer, MqttPort);
  this->mqtt->setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
}

void MQTT::WifiOnEvent(WiFiEvent_t event) {
    if (Config->GetDebugLevel()>=4) {Serial.printf("[WiFi-event] event: %d\n", event);}

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            dbg.println("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            dbg.println("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            dbg.println("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            dbg.println("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            dbg.println("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            dbg.println("Disconnected from WiFi access point");
            this->ConnectStatusWifi = false;
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            dbg.println("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            dbg.printf("WiFi connected with local IP: %s\n", WiFi.localIP().toString().c_str());
            this->ipadresse = WiFi.localIP();
            this->ConnectStatusWifi = true;
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            dbg.println("Lost IP address and IP address is reset to 0");
            this->ConnectStatusWifi = false;
            this->ipadresse = (0,0,0,0);
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            dbg.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            dbg.println("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            dbg.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            dbg.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            dbg.println("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            dbg.println("WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            dbg.println("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            dbg.println("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            dbg.println("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            dbg.println("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            dbg.println("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            dbg.println("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            dbg.println("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            dbg.println("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            dbg.println("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            dbg.println("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            dbg.println("Ethernet disconnected");
            this->ConnectStatusWifi = false;
            this->ipadresse = (0,0,0,0);
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            if (!this->ConnectStatusWifi) {
              dbg.printf("ETH MAC: %s, IPv4: %s, %s, Mbps: %d\n", 
                ETH.macAddress().c_str(), 
                ETH.localIP().toString().c_str(),
                (ETH.fullDuplex()?"FULL_DUPLEX":"HALF_DUPLEX"),
                ETH.linkSpeed()
              );
              this->ipadresse = ETH.localIP();
              this->ConnectStatusWifi = true;
            }
            break;
        default: break;
    }
}
/*######################################
return LanShield parameter tuple 
########################################*/
eth_shield_t* MQTT::GetEthShield(String ShieldName) {
  for(uint8_t i=0; i<this->lan_shields.size(); i++) {
    if(this->lan_shields.at(i).name == ShieldName) {
      return &this->lan_shields.at(i);
      break;
    }
  }
  return &this->lan_shields.at(0);
}

void MQTT::WaitForConnect() {
  while (!this->ConnectStatusWifi)
    delay(100);
    //yield();
}

void MQTT::reconnect() {
  char topic[50];
  char LWT[50];
  memset(&LWT[0], 0, sizeof(LWT));
  memset(&topic[0], 0, sizeof(topic));
  
  if (Config->UseRandomMQTTClientID()) { 
    snprintf (topic, sizeof(topic), "%s-%s", this->mqtt_root.c_str(), String(random(0xffff)).c_str());
  } else {
    snprintf (topic, sizeof(topic), "%s-%08X", this->mqtt_root.c_str(), ESP_getChipId());
  }
  snprintf(LWT, sizeof(LWT), "%s/state", this->mqtt_root.c_str());
  
  dbg.printf("Attempting MQTT connection as %s \n", topic);
  
  if (this->mqtt->connect(topic, Config->GetMqttUsername().c_str(), Config->GetMqttPassword().c_str(), LWT, true, false, "Offline")) {
    dbg.println("connected... ");
    // Once connected, publish basics ...
    this->Publish_IP();
    this->Publish_String("version", Config->GetReleaseName(), false);
    this->Publish_String("state", "Online", false); //LWT reset
    
    // ... and resubscribe if needed
    for (uint8_t i=0; i< this->subscriptions->size(); i++) {
      this->mqtt->subscribe(this->subscriptions->at(i).c_str()); 
      dbg.print("MQTT Subscribed to: "); dbg.println(this->subscriptions->at(i).c_str());
    }

  } else {
    dbg.print(F("failed, rc="));
    dbg.print(this->mqtt->state());
    dbg.println(F(" try again in few seconds"));
  }
}

void MQTT::disconnect() {
  this->mqtt->disconnect();
}

void MQTT::callback(char* topic, byte* payload, unsigned int length) {
  if (MyCallback) {
    MyCallback(topic,payload,length);
  }
}

void MQTT::Publish_Bool(const char* subtopic, bool b, bool fulltopic) {
  String s;
  if(b) {s = "1";} else {s = "0";};
  Publish_String(subtopic, s, fulltopic);
}

void MQTT::Publish_Int(const char* subtopic, int number, bool fulltopic) {
  char buffer[20] = {0}; 
  memset(buffer, 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%d", number);
  Publish_String(subtopic, (String)buffer, fulltopic);
}

void MQTT::Publish_Float(const char* subtopic, float number, bool fulltopic) {
  char buffer[10] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%.2f", number);
  Publish_String(subtopic, (String)buffer, fulltopic);
}

void MQTT::Publish_String(const char* subtopic, String value, bool fulltopic) {
  String topic = this->getTopic(String(subtopic), fulltopic);
  
  if (this->mqtt->connected()) {
    this->mqtt->publish((const char*)topic.c_str(), value.c_str(), true);
    if (Config->GetDebugLevel() >=3) {
      dbg.printf("Publish %s: %s \n", topic.c_str(), value.c_str());
    }
  } else { if (Config->GetDebugLevel() >=2) {dbg.println(F("Request for MQTT Publish, but not connected to Broker")); }}
}

String MQTT::getTopic(String subtopic, bool fulltopic) {
  if (!fulltopic) {
      subtopic = this->mqtt_basepath + "/" + this->mqtt_root +  "/" + subtopic;
  }
  return subtopic;
}

void MQTT::Publish_IP() { 
  char buffer[16] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s", this->ipadresse.toString().c_str());
  Publish_String("IP", buffer, false);
}

void MQTT::setCallback(CALLBACK_FUNCTION) {
    this->MyCallback = MyCallback;
}

/*******************************************************
 * subscribe to a special topic (without /# at end)
*******************************************************/
void MQTT::Subscribe(String topic) {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  
  snprintf(buffer, sizeof(buffer), "%s/#", topic.c_str());
  this->subscriptions->push_back(buffer);
  if (this->mqtt->connected()) {
    this->mqtt->subscribe(buffer);
    if (Config->GetDebugLevel() >=3) {
      dbg.print(F("MQTT now subscribed to: ")); 
      dbg.println(buffer);
    }
  }
}

void MQTT::ClearSubscriptions() {
  for ( uint8_t i=0; i< this->subscriptions->size(); i++) {
    if (mqtt->connected()) { 
      mqtt->unsubscribe(this->subscriptions->at(i).c_str()); 
    }
  }
  this->subscriptions->clear();
  this->subscriptions->shrink_to_fit();
}

void MQTT::loop() { 
  if (this->mqtt_root != Config->GetMqttRoot()) {
    if (Config->GetDebugLevel() >=3) {
      dbg.printf("MQTT DeviceName has changed via Web Configuration from %s to %s \n", this->mqtt_root.c_str(), Config->GetMqttRoot().c_str());
      dbg.println(F("Initiate Reconnect"));
    }
    this->mqtt_root = Config->GetMqttRoot();
    if (this->mqtt->connected()) this->mqtt->disconnect();
  }

  if (this->mqtt_basepath != Config->GetMqttBasePath()) {
    if (Config->GetDebugLevel() > 3) {
      dbg.printf("MQTT Basepath has changed via Web Configuration from %s to %s \n", this->mqtt_basepath.c_str(), Config->GetMqttBasePath().c_str());
      dbg.println(F("Initiate Reconnect"));
    }
    this->mqtt_basepath = Config->GetMqttBasePath();
    if (this->mqtt->connected()) this->mqtt->disconnect();
  }

  // WIFI lost, try to reconnect
  if (!Config->GetUseETH() && !this->ConnectStatusWifi) {
    wifiManager->setConfigPortalTimeout(0);
    WiFi.begin();        
  }

  // WIFI ok, MQTT lost
  if (!this->mqtt->connected() && this->ConnectStatusWifi) { 
    if (millis() - mqttreconnect_lasttry > 10000) {
      //espClient = WiFiClient();
      this->reconnect(); 
      this->mqttreconnect_lasttry = millis();
    }
  } else if (this->ConnectStatusWifi) { 
    this->mqtt->loop();
  }

  if (this->mqtt->connected()) {
    this->ConnectStatusMqtt = true;
  } else {
    this->ConnectStatusMqtt = false;
  }

  if (Config->GetDebugLevel() >=4 && millis() - this->last_keepalive > (30 * 1000))  {
    this->last_keepalive = millis();
    
    if (Config->GetDebugLevel() >=4) {
      char buffer[100] = {0};
      memset(buffer, 0, sizeof(buffer));
      
      snprintf(buffer, sizeof(buffer), "%d", ESP.getFreeHeap());
      this->Publish_String("memory", buffer, false);

      snprintf(buffer, sizeof(buffer), "%d", WiFi.RSSI());
      this->Publish_String("rssi", buffer, false);
    }
  }
  
}
