typedef struct { 
    uint8_t StartByte;              // register address
    uint8_t Length;
    std::string MqttTopicName;
    std::string RealName;
    // datentyp
    int value;
  } register_t;

register_t regs[] = {
    {1, 1,"ENGLISH", "Settings",0},
    {2, 1,"SPANISH", "Ajustes",0},
    {3, 1,"FRENCH", "Paramètres",0}
};
