#ifndef SOLAXMODBUS_H
#define SOLAXMODBUS_H

#include "commonlibs.h"
#include "register.h"
#include "baseconfig.h"
#include "mqtt.h"
#include <vector>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ArduinoQueue.h>
#include <HardwareSerial.h>

#define DEBUGMODE

class modbus {

  typedef struct { 
      String Name;
      String RealName;
      String value;
  } reg_t;

  typedef struct { 
      String Name;
      bool active;
  } itemconfig_t;

  typedef struct {
    String command = "";
    std::vector<byte> request; 
  } subscription_t;


  #define RS485Transmit    HIGH
  #define RS485Receive     LOW

  public:
    modbus();
    void                    init();
    void                    StoreJsonConfig(String* json);
    void                    StoreJsonItemConfig(String* json);
    void                    LoadJsonConfig(bool firstrun);
    void                    LoadJsonItemConfig();

    void                    loop();

    const String&           GetInverterType()   const {return InverterType;}

    void                    enableMqtt(MQTT* object);
    void                    GetWebContentConfig(AsyncResponseStream *response);
    void                    GetWebContentItemConfig(AsyncResponseStream *response);
    void                    GetWebContentRawData(AsyncResponseStream *response);
    void                    GetWebContentActiveLiveData(AsyncResponseStream *response);
    String                  GetInverterSN();
    String                  GetLiveDataAsJson();
    void                    SetItemActiveStatus(String item, bool newstate);
    void                    ReceiveMQTT(String topic, int msg);

  private:
    uint8_t                 pin_RX;               // Serial Receive pin
    uint8_t                 pin_TX;               // Serial Transmit pin
    uint8_t                 pin_RTS;              // Direction control pin
    uint8_t                 ClientID;             // 0x01
    uint32_t                Baudrate;             // 19200
    uint16_t                TxIntervalLiveData;   // 5
    uint16_t                TxIntervalIdData;     // 3600
    String                  InverterType;         //Solax-X1

    unsigned long           LastTxLiveData = 0;
    unsigned long           LastTxIdData = 0;
    unsigned long           LastTxInverter = 0;

    std::vector<byte>*      DataFrame;            // storing read results as hexdata to parse
    std::vector<reg_t>*     InverterIdData;       // storing readable results
    std::vector<reg_t>*     InverterLiveData;     // storing readable results
    std::vector<itemconfig_t>* ActiveItems;       // configured active Modbus Items
    std::vector<String>*    AvailableInverters;   // available inverters from JSON
    std::vector<subscription_t>* Setters;         // available set Options from JSON register 

    MQTT*                   mqtt = NULL;
    
    String                  PrintHex(byte num);
    String                  PrintDataFrame(std::vector<byte>* frame);
    String                  PrintDataFrame(byte* frame, uint8_t len);
    uint16_t                Calc_CRC(uint8_t* message, uint8_t len);
    int                     JsonPosArrayToInt(JsonArray posArray, JsonArray posArray2);
    void                    QueryLiveData();
    void                    QueryIdData();
    void                    QueryQueueToInverter();
    void                    ReceiveReadData();
    bool                    ReceiveSetData(std::vector<byte>* SendHexFrame);
    void                    ParseData();
    void                    LoadInvertersFromJson();
    void                    LoadInverterConfigFromJson();
    void                    GenerateMqttSubscriptions();
    String                  GetMqttSetTopic(String command);

    // inverter config, in sync with register.h ->config
    ArduinoQueue<std::vector<byte>>* ReadQueue;
    ArduinoQueue<std::vector<byte>>* SetQueue;

    std::vector<std::vector<byte>>*  Conf_RequestLiveData;
    std::vector<byte>*      Conf_RequestIdData;
		uint8_t                 Conf_ClientIdPos;
    uint8_t                 Conf_LiveDataStartsAtPos;
		uint8_t                 Conf_IdDataStartsAtPos;
		uint8_t                 Conf_LiveDataErrorPos;
		byte                    Conf_LiveDataErrorCode;
		uint8_t                 Conf_IdDataErrorPos;
		byte                    Conf_IdDataErrorCode;
		uint8_t                 Conf_LiveDataSuccessPos;
		byte                    Conf_LiveDataSuccessCode;
		uint8_t                 Conf_IdDataSuccessPos;
		byte                    Conf_IdDataSuccessCode;
    byte                    Conf_LiveDataFunctionCode;
    byte                    Conf_IdDataFunctionCode;
    uint8_t                 Conf_LiveDataFunctionCodePos;
    uint8_t                 Conf_IdDataFunctionCodePos;

    bool                    Conf_EnableOpenWBTopic;
    bool                    Conf_EnableSetters;

    byte                    String2Byte(String s);

    std::vector<byte>*      SaveIdDataframe;
    std::vector<byte>*      SaveLiveDataframe;

    HardwareSerial*         RS485Serial;

};

extern modbus* mb;

#endif
