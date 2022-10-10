typedef struct { 
    uint8_t StartByte;              // register address
    uint8_t Length;
    String MqttTopicName;
    String RealName;
    char Datatype;               // S = String, I = Integer
    void* value;
  } register_t;

register_t regs[] = {
    {0,   1,"GridVoltage",    "Grid Voltage",       "I", 0},
    {1,   1,"GridCurrent",    "Grid Current",       "I", 0},
    {2,   1,"GridPower",      "Grid Power",         "I", 0},
    {3,   1,"PvVoltage1",     "PV Voltage 1",       "I", 0},
    {4,   1,"PvVoltage2",     "PV Voltage 2",       "I", 0},
    {5,   1,"PvCurrent1",     "PV Current 1",       "I", 0},
    {6,   1,"PvCurrent2",     "PV Current 2",       "I", 0},
    {7,   1,"GridFrequency",  "Grid Frequency",     "I", 0}
};
