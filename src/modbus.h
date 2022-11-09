#ifndef SOLAXMODBUS_H
#define SOLAXMODBUS_H

#include "commonlibs.h"
#include "register.h"
#include "baseconfig.h"
#include "mqtt.h"
#include <vector>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

class modbus {

  typedef struct { 
      String Name;
      String RealName;
      String value;
  } reg_t;


  public:
    modbus();
    void                    init();
    void                    StoreJsonConfig(String* json);
    void                    LoadJsonConfig();

    void                    loop();

    const String&           GetInverterType()   const {return InverterType;}

    void                    enableMqtt(MQTT* object);
    void                    GetWebContentConfig(WM_WebServer* server);
    void                    GetWebContentLiveData(WM_WebServer* server);
    String                  GetInverterSN();
    String                  GetLiveDataAsJson();

  private:
    uint8_t                 ClientID;     // 0x01
    uint32_t                Baudrate;     // 19200
    uint16_t                TxIntervalLiveData;   // 5
    uint16_t                TxIntervalIdData; // 3600
    String                  InverterType; //Solax-X1

    unsigned long           LastTxLiveData = 0;
    unsigned long           LastTxIdData = 0;

    std::vector<reg_t>*     InverterIdData;
    std::vector<reg_t>*     InverterLiveData;
    std::vector<String>*    AvailableInverters;
    MQTT*                   mqtt = NULL;
    
    String                  PrintHex(byte num);
    String                  PrintDataFrame(std::vector<byte>* frame);
    String                  PrintDataFrame(byte* frame, uint8_t len);
    uint16_t                Calc_CRC(uint8_t* message, uint8_t len);
    void                    QueryLiveData();
    void                    QueryIdData();
    void                    ReceiveData();
    void                    LoadInvertersFromJson();
    void                    LoadInverterConfigFromJson();

    // inverter config, in sync with register.h ->config
    std::vector<byte>*      Conf_RequestLiveData;
    std::vector<byte>*      Conf_RequestIdData;
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

    byte                    String2Byte(String s);

    HardwareSerial*         mySerial;

};

extern modbus* mb;

#endif
