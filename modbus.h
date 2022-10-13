#ifndef SOLAXMODBUS_H
#define SOLAXMODBUS_H

#include "commonlibs.h"
#include "register.h"
#include <vector>
#include <ArduinoJson.h>

class modbus {

  typedef struct { 
      const char* MqttTopicName;
      const char* RealName;
      void* value;
  } reg_t;


  public:
    modbus();
    void                  init(uint8_t clientid);
    
    void                  loop();

    const uint8_t& GetClientID()      const {return ClientID;}
    const uint8_t& GetBaudrate()      const {return Baudrate;}
    const uint8_t& GetTxInterval()    const {return TxInterval;}
    

    void                    setBaudrate(int baudrate);
    void                    setTxInterval(int TxInterval);
    
  private:
    uint8_t                 ClientID;
    int                     Baudrate;
    int                     TxInterval; // in seconds
    unsigned long           LastTx = 0;
    std::vector<reg_t>*     InverterData;
    
    String                  PrintHex(byte num);
    uint16_t                Calc_CRC(uint8_t* message, uint8_t len);

    void                    QueryLiveData();
    void                    QueryIdData();
    void                    ReceiveData();



};




#endif
