/*
Anemometer with a RS485 wind sensor

from an idea of https://arduino.stackexchange.com/questions/62327/cannot-read-modbus-data-repetitively
https://www.cupidcontrols.com/2015/10/software-serial-modbus-master-over-rs485-transceiver/

_________________________________________________________________
|                                                               |
|       author : Philippe de Craene <dcphilippe@yahoo.fr        |
|       Any feedback is welcome                                 |
                                                                |
_________________________________________________________________

*/
#include "commonlibs.h"
#include "modbus.h"
#include "baseconfig.h"
#include "mqtt.h"
#include "MyWebServer.h"

modbus* mb = NULL;
BaseConfig* Config = NULL;
MQTT* mqtt = NULL;
MyWebServer* mywebserver = NULL;


void myMQTTCallBack(char* topic, byte* payload, unsigned int length) {
  String msg;
  Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");

  for (int i = 0; i < length; i++) {
    msg.concat((char)payload[i]);
  }
  Serial.print("Message: "); Serial.println(msg.c_str());

  if (strstr(topic, "/raw") ||  strstr(topic, "/level") ||  strstr(topic, "/mem") ||  strstr(topic, "/rssi")) {
    /*SensorMeldungen - ignore!*/
  }
  else {
    
  }
}

void setup() {
  // Start the built-in serial port, for Serial Monitor
  Serial.begin(115200);
  Serial.println("Start of Solax MQTT Gateway"); 

  Serial.println("Starting BaseConfig");
  Config = new BaseConfig();
  //String cfgjson = "{'mqttroot':'solax', 'mqttserver': '192.168.10.10','mqttport':'1883', 'mqttuser':'', 'mqttpass':'', 'UseRandomClientID':'', 'keepalive':'', 'debuglevel':''}";
  //String cfgjson = "{ 'mqttroot': 'solax', 'mqttserver': '192.168.10.10', 'mqttport': '1883', 'UseRandomClientID': 'none', 'keepalive': 0, 'debuglevel': 0, 'count': 6}";
  //Config->StoreJsonConfig(&cfgjson);

  Serial.println("Starting Wifi and MQTT");
  mqtt = new MQTT(Config->GetMqttServer().c_str(), Config->GetMqttPort(), Config->GetMqttRoot().c_str());
  mqtt->setCallback(myMQTTCallBack);

  mb = new modbus();
  mb->init(0x01, 19200);
  mb->setTxInterval(5);

  Serial.println("Starting WebServer");
  mywebserver = new MyWebServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  mqtt->loop();
  mywebserver->loop();
  mb->loop(); 
}
