#include "mqtt.h"

MQTT::MQTT(AsyncWebServer* server, DNSServer *dns, const char* MqttServer, uint16_t MqttPort, String MqttBasepath, String MqttRoot, char* APName, char* APpassword): 
server(server), dns(dns), mqtt_root(MqttRoot), mqtt_basepath(MqttBasepath) { 
  
  this->subscriptions = new std::vector<String>{};

  espClient = WiFiClient();
  WiFi.mode(WIFI_STA); 
  this->mqtt = new PubSubClient();
  
  this->wifiManager = new AsyncWiFiManager(server, dns);

  if (Config->GetDebugLevel() >=4) wifiManager->setDebugOutput(true); 
    else wifiManager->setDebugOutput(false); 

  wifiManager->setConnectTimeout(60);
  wifiManager->setConfigPortalTimeout(300);
  WiFi.setHostname(this->mqtt_root.c_str());
  Serial.println("WiFi Start");
  
//  wifiManager->startConfigPortal("OnDemandAP");

  if (!wifiManager->autoConnect(APName, APpassword) ) {
    Serial.println("failed to connect and start configPortal");
    wifiManager->startConfigPortal(APName, APpassword);
  }
  
  Serial.printf("WiFi connected with local IP: %s\n", WiFi.localIP().toString().c_str());
  //WiFi.printDiag(Serial);

  Serial.printf("Starting MQTT (%s:%d)\n", MqttServer, MqttPort);
  this->mqtt->setClient(espClient);
  this->mqtt->setServer(MqttServer, MqttPort);
  this->mqtt->setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
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
  
  Serial.printf("Attempting MQTT connection as %s \n", topic);
  
  if (this->mqtt->connect(topic, Config->GetMqttUsername().c_str(), Config->GetMqttPassword().c_str(), LWT, true, false, "Offline")) {
    Serial.println("connected... ");
    // Once connected, publish basics ...
    this->Publish_IP();
    this->Publish_String("state", "Online", false); //LWT reset
    
    // ... and resubscribe if needed
    for (uint8_t i=0; i< this->subscriptions->size(); i++) {
      this->mqtt->subscribe(this->subscriptions->at(i).c_str()); 
      Serial.print("MQTT Subscribed to: "); Serial.println(this->subscriptions->at(i).c_str());
    }

  } else {
    Serial.print(F("failed, rc="));
    Serial.print(this->mqtt->state());
    Serial.println(F(" try again in few seconds"));
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
      Serial.printf("Publish %s: %s \n", topic.c_str(), value.c_str());
    }
  } else { if (Config->GetDebugLevel() >=2) {Serial.println(F("Request for MQTT Publish, but not connected to Broker")); }}
}

String MQTT::getTopic(String subtopic, bool fulltopic) {
  if (!fulltopic) {
      subtopic = this->mqtt_basepath + "/" + this->mqtt_root +  "/" + subtopic;
  }
  return subtopic;
}

void MQTT::Publish_IP() {
  char buffer[15] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%s", WiFi.localIP().toString().c_str());
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
      Serial.print(F("MQTT now subscribed to: ")); 
      Serial.println(buffer);
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
      Serial.printf("MQTT DeviceName has changed via Web Configuration from %s to %s \n", this->mqtt_root.c_str(), Config->GetMqttRoot().c_str());
      Serial.println(F("Initiate Reconnect"));
    }
    this->mqtt_root = Config->GetMqttRoot();
    if (this->mqtt->connected()) this->mqtt->disconnect();
  }

  if (this->mqtt_basepath != Config->GetMqttBasePath()) {
    if (Config->GetDebugLevel() > 3) {
      Serial.printf("MQTT Basepath has changed via Web Configuration from %s to %s \n", this->mqtt_basepath.c_str(), Config->GetMqttBasePath().c_str());
      Serial.println(F("Initiate Reconnect"));
    }
    this->mqtt_basepath = Config->GetMqttBasePath();
    if (this->mqtt->connected()) this->mqtt->disconnect();
  }

  // WIFI lost, try to reconnect
  if (WiFi.status() != WL_CONNECTED) {
    if (Config->GetDebugLevel() > 1) {
      Serial.println("WIFI lost, try to reconnect...");
    }
    wifiManager->setConfigPortalTimeout(0);
    while (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      WiFi.begin();        
    }
  }
  
  // WIFI ok, MQTT lost
  if (!this->mqtt->connected() && WiFi.status() == WL_CONNECTED) { 
    if (millis() - mqttreconnect_lasttry > 10000) {
      espClient = WiFiClient();
      this->reconnect(); 
      this->mqttreconnect_lasttry = millis();
    }
  } else if (WiFi.status() == WL_CONNECTED) { 
    this->mqtt->loop();
  }

  if (WiFi.status() == WL_CONNECTED) {
    this->ConnectStatusWifi = true;
  } else {
    this->ConnectStatusWifi = false;
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
