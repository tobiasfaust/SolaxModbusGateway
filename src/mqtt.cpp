/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#include <mqtt.h>

MQTT::MQTT(const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot):
          improvSerial(&Serial),
          mqtt_root(MqttRoot),
          mqtt_basepath(MqttBasepath),
          ConnectStatusWifi(false),
          ConnectStatusMqtt(false) {
  this->subscriptions = new std::vector<String>{};

  WiFi.setHostname(this->mqtt_root.c_str());

#ifdef ESP32
  WiFi.onEvent(std::bind(&MQTT::WifiOnEvent, this, std::placeholders::_1));
#endif

  Config->log(3, "Go into %s Mode", (Config->GetUseETH()?"ETH":"Wifi"));

  ImprovTypes::ChipFamily variant;

#ifdef ESP32
  String variantString = ARDUINO_VARIANT;
#else
  String variantString = "esp8266";
#endif

  if (variantString == "esp32s3") {
      variant = ImprovTypes::ChipFamily::CF_ESP32_S3;
  } else if (variantString == "esp32c3") {
      variant = ImprovTypes::ChipFamily::CF_ESP32_C3;
  } else if (variantString == "esp32s2") {
      variant = ImprovTypes::ChipFamily::CF_ESP32_S2;
  } else if (variantString == "esp8266") {
      variant = ImprovTypes::ChipFamily::CF_ESP8266;
  } else {
      variant = ImprovTypes::ChipFamily::CF_ESP32;
  }

  improvSerial.setDeviceInfo(variant,
                             String(GIT_REPO).c_str(),
                             Config->GetReleaseName().c_str(),
                             Config->GetMqttRoot().c_str());
  improvSerial.onImprovError(std::bind(&MQTT::onImprovWiFiErrorCb,
                             this,
                             std::placeholders::_1));

  if (Config->GetUseETH()) {
    #ifdef ESP32
      eth_shield_t* shield = this->GetEthShield(Config->GetLANBoard());

      // ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
      ETH.begin(shield->PHY_ADDR,
                shield->PHY_POWER,
                shield->PHY_MDC,
                shield->PHY_MDIO,
                shield->PHY_TYPE,
                shield->CLK_MODE);

      this->WaitForConnect();
    #endif

  } else {
    // use Wifi
    improvSerial.ConnectToWifi();
  }

  if (Config->GetDebugLevel() >=4) WiFi.printDiag(dbg);

  Config->log(1, "Initializing MQTT (%s:%d)", Config->GetMqttServer().c_str(), Config->GetMqttPort());
  espClient = WiFiClient();

  PubSubClient::setClient(espClient);
  PubSubClient::setServer(Config->GetMqttServer().c_str(), Config->GetMqttPort());
}

void MQTT::onImprovWiFiErrorCb(ImprovTypes::Error err) {
  if (err == ImprovTypes::Error::ERROR_WIFI_DISCONNECTED) {
    this->disconnect();
  }
  if (err == ImprovTypes::Error::ERROR_WIFI_CONNECT_GIVEUP) {
    Serial.println("Giving up on connecting to WiFi, restart the device");
    ESP.restart();
  }
}

#ifdef ESP32
void MQTT::WifiOnEvent(WiFiEvent_t event) {
    Config->log(4, "[WiFi-event] event: %d", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY:
            Config->log(1, "WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            Config->log(1, "Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            Config->log(1, "WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            Config->log(1, "WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Config->log(1, "Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Config->log(1, "Disconnected from WiFi access point");
            this->ConnectStatusWifi = false;
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            Config->log(1, "Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Config->log(1, "WiFi connected with local IP: %s", WiFi.localIP().toString().c_str());
            this->ipadresse = WiFi.localIP();
            this->ConnectStatusWifi = true;
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Config->log(1, "Lost IP address and IP address is reset to 0");
            this->ConnectStatusWifi = false;
            this->ipadresse = (0, 0, 0, 0);
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Config->log(1, "WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            Config->log(1, "WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Config->log(1, "WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            Config->log(1, "WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Config->log(1, "WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Config->log(1, "WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            Config->log(1, "Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            Config->log(1, "Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            Config->log(1, "Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            Config->log(1, "Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            Config->log(1, "AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Config->log(1, "STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            Config->log(1, "Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            Config->log(1, "Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            Config->log(1, "Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            Config->log(1, "Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Config->log(1, "Ethernet disconnected");
            this->ConnectStatusWifi = false;
            this->ipadresse = (0, 0, 0, 0);
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            if (!this->ConnectStatusWifi) {
              Config->log(1, "ETH MAC: %s, IPv4: %s, %s, Mbps: %d",
                ETH.macAddress().c_str(),
                ETH.localIP().toString().c_str(),
                (ETH.fullDuplex()?"FULL_DUPLEX":"HALF_DUPLEX"),
                ETH.linkSpeed());
              this->ipadresse = ETH.localIP();
              this->ConnectStatusWifi = true;
            }
            break;
        default: break;
    }
}
#endif

/*######################################
return LanShield parameter tuple 
########################################*/
eth_shield_t* MQTT::GetEthShield(String ShieldName) {
  for (uint8_t i = 0; i < this->lan_shields.size(); i++) {
    if (this->lan_shields.at(i).name == ShieldName) {
      return &this->lan_shields.at(i);
      break;
    }
  }
  return &this->lan_shields.at(0);
}

void MQTT::WaitForConnect() {
  while (!this->ConnectStatusWifi)
    delay(100);
    Config->log(1, "Wait for connect");
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

  Config->log(1, "Attempting MQTT connection as %s ", topic);

  if (PubSubClient::connect(topic,
                            Config->GetMqttUsername().c_str(),
                            Config->GetMqttPassword().c_str(),
                            LWT,
                            true,
                            false,
                            "Offline")) {
    Config->log(1, "connected... ");
    // Once connected, publish basics ...
    this->Publish_IP();
    this->Publish_String("ssid", WiFi.SSID(), false);
    this->Publish_String("version", Config->GetReleaseName(), false);
    this->Publish_String("state", "Online", false);  // LWT reset

    // ... and resubscribe if needed
    for (uint8_t i=0; i< this->subscriptions->size(); i++) {
      PubSubClient::subscribe(this->subscriptions->at(i).c_str());
      Config->log(1, "MQTT resubscribed to: %s", this->subscriptions->at(i).c_str());
    }

  } else {
    Config->log(1, "failed, rc=%d - Trying again in 5 seconds", PubSubClient::state());
  }
}

void MQTT::disconnect() {
  PubSubClient::disconnect();
}

void MQTT::Publish_Bool(const char* subtopic, bool b, bool fulltopic) {
  String s("");
  if (b) {s = "1";} else {s = "0";}
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

  if (PubSubClient::connected()) {
    PubSubClient::publish((const char*)topic.c_str(), value.c_str(), true);
    Config->log(3, "Publish %s: %s ", topic.c_str(), value.c_str());
  } else { Config->log(2, "Request for MQTT Publish, but not connected to Broker"); }
}

String MQTT::getTopic(String subtopic, bool fulltopic) {
  if (!fulltopic) {
    return std::move(this->mqtt_basepath + "/" + this->mqtt_root +  "/" + subtopic);
  }
  return std::move(subtopic);
}

void MQTT::Publish_IP() {
  char buffer[16] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s", this->ipadresse.toString().c_str());
  Publish_String("IP", buffer, false);
}

/*******************************************************
 * subscribe to a special topic (without /# at end)
*******************************************************/
void MQTT::Subscribe(String topic) {
  this->subscriptions->push_back(topic);
  if (PubSubClient::connected()) {
    PubSubClient::subscribe(topic.c_str());
    Config->log(3, "MQTT now subscribed to: %s", topic.c_str());
  }
}

bool MQTT::UnSubscribe(String topic) {
  bool ret = false;
  for (uint8_t i=0; i< this->subscriptions->size(); i++) {
    if (topic == this->subscriptions->at(i)) {
      if (PubSubClient::connected()) {
        PubSubClient::unsubscribe(this->subscriptions->at(i).c_str());
      }
      Config->log(3, "MQTT unsubscribed from: %s", this->subscriptions->at(i).c_str());
      this->subscriptions->erase(this->subscriptions->begin()+i);
      ret = true;
      break;
    }
  }
  return ret;
}

void MQTT::ClearSubscriptions() {
  for (uint8_t i = 0; i < this->subscriptions->size(); i++) {
    if (PubSubClient::connected()) {
      PubSubClient::unsubscribe(this->subscriptions->at(i).c_str());
    }
  }
  this->subscriptions->clear();
  this->subscriptions->shrink_to_fit();
}

void MQTT::loop() {
  improvSerial.loop();

  #ifdef ESP8266
    if (WiFi.status() == WL_CONNECTED) {
      this->ConnectStatusWifi = true;
      this->ipadresse = WiFi.localIP();
    } else {
      this->ConnectStatusWifi = false;
      this->ipadresse = (0, 0, 0, 0);
    }
  #endif

  if (this->mqtt_root != Config->GetMqttRoot()) {
    Config->log(3, "MQTT DeviceName has changed via Web Configuration from %s to %s ",
                    this->mqtt_root.c_str(),
                    Config->GetMqttRoot().c_str());
    Config->log(3, "Initiate Reconnect");

    this->mqtt_root = Config->GetMqttRoot();
    if (PubSubClient::connected()) PubSubClient::disconnect();
  }

  if (this->mqtt_basepath != Config->GetMqttBasePath()) {
    Config->log(3, "MQTT Basepath has changed via Web Configuration from %s to %s ",
                    this->mqtt_basepath.c_str(),
                    Config->GetMqttBasePath().c_str());
    Config->log(3, "Initiate Reconnect");

    this->mqtt_basepath = Config->GetMqttBasePath();
    if (PubSubClient::connected()) PubSubClient::disconnect();
  }

  // WIFI ok, MQTT lost
  if (!PubSubClient::connected() && this->ConnectStatusWifi) {
    if (millis() - mqttreconnect_lasttry > 10000) {
      this->reconnect();
      this->mqttreconnect_lasttry = millis();
    }
  } else if (this->ConnectStatusWifi) {
    PubSubClient::loop();
  }

  if (PubSubClient::connected()) {
    this->ConnectStatusMqtt = true;
  } else {
    this->ConnectStatusMqtt = false;
  }

  if (Config->GetDebugLevel() >=4 && millis() - this->last_keepalive > (30 * 1000))  {
    // send messages for debugging every 30 seconds
    this->last_keepalive = millis();

    if (Config->GetDebugLevel() >=4) {
      char buffer[100] = {0};
      memset(buffer, 0, sizeof(buffer));

      snprintf(buffer, sizeof(buffer), "%d", ESP.getFreeHeap() / 1024);
      this->Publish_String("memory", buffer, false);

      snprintf(buffer, sizeof(buffer), "%d", WiFi.RSSI());
      this->Publish_String("rssi", buffer, false);

      uint64_t uptimeMicroSeconds = esp_timer_get_time();
      uint64_t uptimeSeconds = uptimeMicroSeconds / 1000000;
      this->Publish_Int("uptime", uptimeSeconds, false);
    }
  }
}
