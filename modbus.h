#ifndef SOLAXMODBUS_H
#define SOLAXMODBUS_H

class modbus {

  public:
    modbus();
    void                  init(uint8_t clientid);
    
    void                  loop();

    const uint8_t& GetClientID()         const {return ClientID;}
    const uint8_t& GetBaudrate()      const {return Baudrate;}
    const uint8_t& GetTxInterval()      const {return TxInterval;}
    

    void                  setBaudrate(int baudrate);
    void                  setTxInterval(int TxInterval);
    
  private:
    uint8_t              ClientID;
    int                     Baudrate;
    int                     TxInterval;
    unsigned long   LastTx = 0;
    
    
    std::string         Hex2String(uint8_t num);
    uint16_t            Calc_CRC(uint8_t* message, uint8_t len);

    void                  QueryLiveData();
    void                  ReceiveLiveData();

};

#endif
