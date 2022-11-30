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

class modbus {

  typedef struct { 
      String Name;
      String RealName;
      String value;
  } reg_t;

  typedef struct { 
      String Name;
      bool value;
  } itemconfig_t;

  public:
    modbus();
    void                    init();
    void                    StoreJsonConfig(String* json);
    void                    StoreJsonItemConfig(String* json);
    void                    LoadJsonConfig();
    void                    LoadJsonItemConfig();

    void                    loop();

    const String&           GetInverterType()   const {return InverterType;}

    void                    enableMqtt(MQTT* object);
    void                    GetWebContentConfig(WM_WebServer* server);
    void                    GetWebContentItemConfig(WM_WebServer* server);
    String                  GetInverterSN();
    String                  GetLiveDataAsJson();
    void                    SetItemActiveStatus(String item, bool newstate);

  private:
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
    MQTT*                   mqtt = NULL;
    
    String                  PrintHex(byte num);
    String                  PrintDataFrame(std::vector<byte>* frame);
    String                  PrintDataFrame(byte* frame, uint8_t len);
    uint16_t                Calc_CRC(uint8_t* message, uint8_t len);
    void                    QueryLiveData();
    void                    QueryIdData();
    void                    QueryQueueToInverter();
    void                    ReceiveData();
    void                    ParseData();
    void                    LoadInvertersFromJson();
    void                    LoadInverterConfigFromJson();

    // inverter config, in sync with register.h ->config
    ArduinoQueue<std::vector<byte>>* RequestQueue;

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

    byte                    String2Byte(String s);

    HardwareSerial*         mySerial;

};

extern modbus* mb;

#endif
