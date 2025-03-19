/*
Solar Inverter Modbus-RTU Gateway to MQTT

_________________________________________________________________
|                                                               |
|       Copyright [2024] Tobias Faust <tobias.faust@gmx.net     |
|       Any feedback is welcome                                 |
|                                                               |
_________________________________________________________________
 
*/

#include <commonlibs.h>
#include <modbus.h>
#include <baseconfig.h>
#include <mqtt.h>
#include <MyWebServer.h>

AsyncWebServer server(80);
DNSServer dns;

modbus* mb = NULL;
BaseConfig* Config = NULL;
MQTT* mqtt = NULL;
MyWebServer* mywebserver = NULL;

void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String msg;
  Config->logN(3, "Message arrived [%s]", topic);

  for (unsigned int i = 0; i < length; i++) {
    msg.concat(static_cast<char>(payload[i]));
  }

  Config->logN(3, "Message: %s", msg.c_str());

  mb->ReceiveMQTT(topic, msg);
}

void setup() {
  Serial.begin(115200);
  Config = new BaseConfig();

  #ifndef USE_WEBSERIAL
    Serial.begin(115200,
                 SERIAL_8N1,
                 Config->GetSerialRx(),
                 Config->GetSerialTx());  // RX, TX, zb.: 33, 32
    Serial.println("");
    Serial.println("ready");
  #endif

  #ifdef USE_WEBSERIAL
    WebSerial.onMessage([](const String& msg) { Serial.println(msg); });
    WebSerial.begin(&server);
    WebSerial.setBuffer(100);
  #endif

  Config->logN(1, "Start of Modbus-RTU MQTT Gateway");
  Config->logN(1, "Starting BaseConfig");

  Config->logN(1, "Starting Wifi and MQTT");
  mqtt = new MQTT(Config->GetMqttServer().c_str(),
                    Config->GetMqttPort(),
                    Config->GetMqttBasePath().c_str(),
                    Config->GetMqttRoot().c_str());
  mqtt->setCallback(myMQTTCallBack);

  mb = new modbus();
  mb->enableMqtt(mqtt);

  Config->logN(1, "attempting to start WebServer");
  mywebserver = new MyWebServer(&server, &dns);
}

void loop() {
  mqtt->loop();
  mywebserver->loop();
  mb->loop();

  #ifdef USE_WEBSERIAL
    WebSerial.loop();
  #endif
}
