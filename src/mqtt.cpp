#include "mqtt.h"

MQTT::MQTT(const char* server, uint16_t port, String basepath, String root) { 
  this->mqtt_basepath = basepath;
  this->mqtt_root = root;
  espClient = WiFiClient();
  WiFi.mode(WIFI_STA); 
  this->mqtt = new PubSubClient();
  WiFiManager wifiManager;

  
  if (Config->GetDebugLevel() >=4) wifiManager.setDebugOutput(true); 
    else wifiManager.setDebugOutput(false); 

  wifiManager.setTimeout(300);
  Serial.println("WiFi Start");
  //wifi_station_set_hostname(mqtt_root.c_str());
  //SetHostName(mqtt_root.c_str()); //TODO
  
  if (!wifiManager.autoConnect(mqtt_root.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
  
  Serial.print("WiFi connected with local IP: ");
  Serial.println(WiFi.localIP());
  //WiFi.printDiag(Serial);

  Serial.print("Starting MQTT (");Serial.print(server); Serial.print(":");Serial.print(port);Serial.println(")");
  this->mqtt->setClient(espClient);
  this->mqtt->setServer(server, port);
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
    snprintf (topic, sizeof(topic), "%s-%08X", this->mqtt_root.c_str(), WIFI_getChipId());
  }
  snprintf(LWT, sizeof(LWT), "%s/state", this->mqtt_root.c_str());
  
  Serial.printf("Attempting MQTT connection as %s \n", topic);
  
  if (this->mqtt->connect(topic, Config->GetMqttUsername().c_str(), Config->GetMqttPassword().c_str(), LWT, true, false, "Offline")) {
    Serial.println("connected... ");
    // Once connected, publish basics ...
    this->Publish_IP();
    this->Publish_String("state", "Online", false); //LWT reset
    
    // ... and resubscribe if needed
    //snprintf (topic, sizeof(topic), "%s/%s/#", this->mqtt_basepath.c_str(), this->mqtt_root.c_str());
    //this->mqtt->subscribe(topic);
    //Serial.print(F("MQTT Subscribed to: ")); Serial.println(FPSTR(topic));

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

String MQTT::GetRoot() {
  return mqtt_root;
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
  Publish_String(subtopic, buffer, fulltopic);
}

void MQTT::Publish_Float(const char* subtopic, float number, bool fulltopic) {
  char buffer[10] = {0};
  memset(&buffer[0], 0, sizeof(buffer));
  snprintf(buffer, sizeof(buffer), "%.2f", number);
  Publish_String(subtopic, buffer, fulltopic);
}

void MQTT::Publish_String(const char* subtopic, String value, bool fulltopic) {
  char topic[50] = {0};
  memset(topic, 0, sizeof(topic));
  if (fulltopic) {
    snprintf(topic, sizeof(topic), "%s", subtopic);
  } else {
    snprintf(topic, sizeof(topic), "%s/%s/%s", this->mqtt_basepath.c_str(), this->mqtt_root.c_str(), subtopic);
  }
  if (this->mqtt->connected()) {
    this->mqtt->publish((const char*)topic, value.c_str(), true);
    if (Config->GetDebugLevel() >=3) {Serial.print(F("Publish ")); Serial.print(FPSTR(topic)); Serial.print(F(": ")); Serial.println(value);}
  } else { if (Config->GetDebugLevel() >=2) {Serial.println(F("Request for MQTT Publish, but not connected to Broker")); }}
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
