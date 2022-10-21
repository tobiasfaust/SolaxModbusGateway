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
    void                    init(uint8_t clientid, uint32_t baudrate);
    
    void                    loop();

    const uint8_t&          GetClientID()     const {return ClientID;}
    const uint32_t&         GetBaudrate()     const {return Baudrate;}

    void                    setBaudrate(int baudrate);
    void                    enableMqtt(MQTT* object);
    
  private:
    uint8_t                 ClientID;
    uint32_t                Baudrate;
    unsigned long           LastTx = 0;
    std::vector<reg_t>*     InverterData;
    
    String                  PrintHex(byte num);
    String                  PrintDataFrame(std::vector<byte>* frame);
    uint16_t                Calc_CRC(uint8_t* message, uint8_t len);
    MQTT*                   mqtt = NULL;
    void                    QueryLiveData();
    void                    QueryIdData();
    void                    ReceiveData();

    HardwareSerial*         mySerial;

};




#endif
