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

AsyncWebServer server(80);
DNSServer dns;

modbus* mb = NULL;
BaseConfig* Config = NULL;
MQTT* mqtt = NULL;
MyWebServer* mywebserver = NULL;


void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String msg;
  if (Config->GetDebugLevel() >=3) {
    dbg.print("Message arrived ["); dbg.print(topic); dbg.print("] ");
  }

  for (unsigned int i = 0; i < length; i++) {
    msg.concat((char)payload[i]);
  }

  if (Config->GetDebugLevel() >=3) {
    dbg.print("Message: "); dbg.println(msg.c_str());
  }

  mb->ReceiveMQTT(topic, atoi(msg.c_str()));
}

void setup() {
  Serial.begin(115200);

  dbg.println("Start of Solar Inverter MQTT Gateway"); 
  dbg.println("Starting BaseConfig");
  Config = new BaseConfig();

  #ifndef USE_WEBSERIAL
    dbg.begin(115200, SERIAL_8N1, Config->GetSerialRx(), Config->GetSerialTx()); // RX, TX, zb.: 33, 32
    dbg.println("");
    dbg.println("ready");
  #endif

  #ifdef USE_WEBSERIAL
    WebSerial.onMessage([](const String& msg) { Serial.println(msg); });
    WebSerial.begin(&server);
    WebSerial.setBuffer(100);
  #endif
  
  dbg.println("Starting Wifi and MQTT");
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

  dbg.println("Starting WebServer");
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
