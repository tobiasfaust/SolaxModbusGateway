/*
Solar Inverter Modbus-RTU Gateway to MQTT

_________________________________________________________________
|                                                               |
|       author : Tobias Faust <tobias.faust@gmx.net             |
|       Any feedback is welcome                                 |
|                                                               |
_________________________________________________________________

*/

#include "commonlibs.h"
#include "modbus.h"
#include "baseconfig.h"
#include "mqtt.h"
#include "MyWebServer.h"
#include <ArduinoQueue.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);
DNSServer dns;

modbus* mb = NULL;
BaseConfig* Config = NULL;
MQTT* mqtt = NULL;
MyWebServer* mywebserver = NULL;
pinMode(this->pin_Relais1, INPUT);
pinMode(this->pin_Relais2, INPUT);

void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String msg;
  if (Config->GetDebugLevel() >=3) {
    Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
  }

  for (unsigned int i = 0; i < length; i++) {
    msg.concat((char)payload[i]);
  }

  if (Config->GetDebugLevel() >=3) {
    Serial.print("Message: "); Serial.println(msg.c_str());
  }

  mb->ReceiveMQTT(topic, atoi(msg.c_str()));
}

void setup() {
  Serial.begin(115200);
  Serial.println("Start of Solar Inverter MQTT Gateway"); 

  Serial.println("Starting BaseConfig");
  Config = new BaseConfig();
  
  Serial.println("Starting Wifi and MQTT");
  mqtt = new MQTT(&server, &dns, 
                    Config->GetMqttServer().c_str(), 
                    Config->GetMqttPort(), 
                    Config->GetMqttBasePath().c_str(), 
                    Config->GetMqttRoot().c_str(),
                    (char*)"AP_ModbusGateway",
                    (char*)"MbMQTTGtw"
                  );
  mqtt->setCallback(myMQTTCallBack);

  mb = new modbus();
  mb->enableMqtt(mqtt);

  Serial.println("Starting WebServer");
  mywebserver = new MyWebServer(&server, &dns);
}

void loop() {
  mqtt->loop();
  mywebserver->loop();
  mb->loop(); 
}
