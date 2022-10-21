#ifndef SOLAXREGISTER_H
#define SOLAXREGISTER_H

/* https://jsonlint.com/ */
const char JSON[] PROGMEM = R"=====(
 {
 "livedata": [{
      "position": [0,1],
      "name": "GridVoltage",
      "realname": "Grid Voltage",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": [2,3],
      "name": "GridCurrent",
      "realname": "Grid Current",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": [4,5],
      "name": "GridPower",
      "realname": "Grid Power",
      "datatype": "integer"
    },
    {
      "position": [6,7],
      "name": "PvVoltage1",
      "realname": "Pv Voltage 1",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": [8,9],
      "name": "PvVoltage2",
      "realname": "Pv Voltage 2",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": [10,11],
      "name": "PvCurrent1",
      "realname": "Pv Current 1",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": [12,13],
      "name": "PvCurrent2",
      "realname": "Pv Current 2",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": [14,15],
      "name": "GridFrequency",
      "realname": "Grid Frequency",
      "datatype": "float",
      "factor": 0.01
    },
    {
      "position": [16,17],
      "name": "Temperature",
      "realname": "Temperature",
      "datatype": "integer"
    },
    {
      "position": [20,21],
      "name": "PowerDC1",
      "realname": "Power DC 1",
      "datatype": "integer"
    },
    {
      "position": [22,23],
      "name": "PowerDC2",
      "realname": "Power DC 2",
      "datatype": "integer"
    }
  ],
  "id": [{
    "position": [0,1,2,3,4,5,6,7,8,9,10,11,12,13],
    "name": "InverterSN",
    "realname": "Inverter SN",
    "datatype": "string"
  }]
}
)=====";

#endif
