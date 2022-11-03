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
      const char* Name;
      const char* RealName;
      void* value;
  } reg_t;


  public:
    modbus();
    void                    init();
    void                    StoreJsonConfig(String* json);
    void                    LoadJsonConfig();

    void                    loop();

    //const uint8_t&          GetClientID()     const {return ClientID;}
    //const uint32_t&         GetBaudrate()     const {return Baudrate;}
    const String&           GetInverterType()   const {return InverterType;}

    //void                    setBaudrate(int baudrate);
    void                    enableMqtt(MQTT* object);
    void                    GetWebContent(WM_WebServer* server);

  private:
    uint8_t                 ClientID;     // 0x01
    uint32_t                Baudrate;     // 19200
    uint16_t                TxIntervalLiveData;   // 5
    uint16_t                TxIntervalIdData; // 3600
    String                  InverterType; //Solax-X1

    unsigned long           LastTxLiveData = 0;
    unsigned long           LastTxIdData = 0;

    std::vector<reg_t>*     InverterData;
    std::vector<String>*    AvailableInverters;
    
    String                  PrintHex(byte num);
    String                  PrintDataFrame(std::vector<byte>* frame);
    uint16_t                Calc_CRC(uint8_t* message, uint8_t len);
    MQTT*                   mqtt = NULL;
    void                    QueryLiveData();
    void                    QueryIdData();
    void                    ReceiveData();
    void                    LoadInvertersFromJson();
    void                    LoadInverterConfigFromJson();


    HardwareSerial*         mySerial;

};

extern modbus* mb;

#endif
