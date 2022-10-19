#ifndef SOLAXREGISTER_H
#define SOLAXREGISTER_H

/* https://jsonlint.com/ */
const char JSON[] PROGMEM = R"=====(
 {
 "livedata": [{
      "position": 0,
      "length": 2,
      "name": "GridVoltage",
      "realname": "Grid Voltage",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": 2,
      "length": 2,
      "name": "GridCurrent",
      "realname": "Grid Current",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": 4,
      "length": 2,
      "name": "GridPower",
      "realname": "Grid Power",
      "datatype": "integer",
      "factor": 1
    },
    {
      "position": 6,
      "length": 2,
      "name": "PvVoltage1",
      "realname": "Pv Voltage 1",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": 8,
      "length": 2,
      "name": "PvVoltage2",
      "realname": "Pv Voltage 2",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": 10,
      "length": 2,
      "name": "PvCurrent1",
      "realname": "Pv Current 1",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": 12,
      "length": 2,
      "name": "PvCurrent2",
      "realname": "Pv Current 2",
      "datatype": "float",
      "factor": 0.1
    },
    {
      "position": 14,
      "length": 2,
      "name": "GridFrequency",
      "realname": "Grid Frequency",
      "datatype": "float",
      "factor": 0.01
    },
    {
      "position": 16,
      "length": 2,
      "name": "Temperature",
      "realname": "Temperature",
      "datatype": "integer"
    },
    {
      "position": 20,
      "length": 2,
      "name": "PowerDC1",
      "realname": "Power DC 1",
      "datatype": "integer",
      "factor": 1
    },
    {
      "position": 22,
      "length": 2,
      "name": "PowerDC2",
      "realname": "Power DC 2",
      "datatype": "integer",
      "factor": 1
    }
  ],
  "id": [{
    "position": 0,
    "length": 14,
    "name": "InverterSN",
    "realname": "Inverter SN",
    "datatype": "string"
  }]
}
)=====";

#endif
