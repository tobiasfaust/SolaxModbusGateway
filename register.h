#ifndef SOLAXREGISTER_H
#define SOLAXREGISTER_H

enum DataType_t {STRING, INTEGER, FLOAT};

typedef struct { 
    uint8_t Position;              // register address
    uint8_t Length;
    String MqttTopicName;
    String RealName;
    DataType_t Datatype;            // S = String, I = Integer, F = Float
    float factor;                   // Umrechnungsfaktor, nur bei Datatype = I,F, default = 1
    void* value;
  } reg_t;

reg_t RegsLiveData[11] = {
    {0,   2,"GridVoltage",    "Grid Voltage",         INTEGER,  0.1,  0},
    {2,   2,"GridCurrent",    "Grid Current",         INTEGER,  0.1,  0},
    {4,   2,"GridPower",      "Grid Power",           INTEGER,    1,  0},
    {6,   2,"PvVoltage1",     "PV Voltage 1",         INTEGER,  0.1,  0},
    {8,   2,"PvVoltage2",     "PV Voltage 2",         INTEGER,  0.1,  0},
    {10,  2,"PvCurrent1",     "PV Current 1",         INTEGER,  0.1,  0},
    {12,  2,"PvCurrent2",     "PV Current 2",         INTEGER,  0.1,  0},
    {14,  2,"GridFrequency",  "Grid Frequency",       INTEGER, 0.01,  0},
    {16,  2,"Temperature",    "Radiator Temperature", INTEGER,    1,  0},
//  {18,  2,"Run Mode",       "Run Mode",             STRING,     1,  0}, 
    {20,  2,"PowerDC1",       "Power DC 1",           INTEGER,    1,  0},
    {22,  2,"PowerDC2",       "Power DC 2",           INTEGER,    1,  0}
};

reg_t RegsIdData[1] = {
    {0,  14,"InverterSN",     "Inverter SerialNumber",STRING,     1,  0}
};

#endif;
