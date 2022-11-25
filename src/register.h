#ifndef SOLAXREGISTER_H
#define SOLAXREGISTER_H

/* https://jsonlint.com/ */
const char JSON[] PROGMEM = R"=====(
{
	"Solax-X1": {
		"config": {
			"RequestLiveData": [
				["#ClientID", "0x04", "0x00", "0x00", "0x00", "0x77"],
				["#ClientID", "0x04", "0x00", "0x78", "0x00", "0x77"]
			],
			"RequestIdData": ["#ClientID", "0x03", "0x00", "0x00", "0x00", "0x14"],
			"LiveDataFunctionCode": "0x04",
			"IdDataFunctionCode": "0x03",
			"LiveDataStartsAtPos": 3,
			"IdDataStartsAtPos": 3,
			"LiveDataErrorPos": 1,
			"LiveDataErrorCode": "0x83",
			"IdDataErrorPos": 1,
			"IdDataErrorCode": "0x84",
			"LiveDataSuccessPos": 1,
			"LiveDataSuccessCode": "0x03",
			"IdDataSuccessPos": 1,
			"IdDataSuccessCode": "0x04"
		},
		"data": {
			"livedata": [{
					"position": [0, 1],
					"name": "GridVoltage",
					"realname": "Grid Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [2, 3],
					"name": "GridCurrent",
					"realname": "Grid Current",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [4, 5],
					"name": "GridPower",
					"realname": "Grid Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [6, 7],
					"name": "PvVoltage1",
					"realname": "Pv Voltage 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [8, 9],
					"name": "PvVoltage2",
					"realname": "Pv Voltage 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [10, 11],
					"name": "PvCurrent1",
					"realname": "Pv Current 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [12, 13],
					"name": "PvCurrent2",
					"realname": "Pv Current 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [14, 15],
					"name": "GridFrequency",
					"realname": "Grid Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [16, 17],
					"name": "Temperature",
					"realname": "Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [20, 21],
					"name": "PowerDC1",
					"realname": "Power DC 1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [22, 23],
					"name": "PowerDC2",
					"realname": "Power DC 2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [40, 41],
					"name": "BatVoltage",
					"realname": "Battery Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [42, 43],
					"name": "BatCurrent",
					"realname": "Battery Current",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [44, 45],
					"name": "BatPower",
					"realname": "Battery Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [48, 49],
					"name": "BatTemp",
					"realname": "Battery Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [53, 54],
					"name": "BatCapacity",
					"realname": "Battery Capacity",
					"datatype": "integer",
					"unit": "%"
				},
				{
					"position": [57, 58, 55, 56],
					"name": "OutputEnergyCharge",
					"realname": "Output Energy Charge",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [61, 61],
					"name": "OutputEnergyChargeToday",
					"realname": "Output Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [63, 64, 65, 66],
					"name": "InputEnergyCharge",
					"realname": "Input Energy Charge",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [67, 68],
					"name": "InputEnergyChargeToday",
					"realname": "Input Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [138, 139, 140, 141],
					"name": "feedinPower",
					"realname": "FeedIn Energy Power to Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [142, 143, 144, 145],
					"name": "feedinEnergyTotal",
					"realname": "FeedIn Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [146, 147, 148, 149],
					"name": "consumedEnergyTotal",
					"realname": "Consumed Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [160, 161],
					"name": "EtodayToGrid",
					"realname": "Today Energy to Grid",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [162, 163, 164, 165],
					"name": "EtotalToGrid",
					"realname": "Total Energy to Grid",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [268, 269, 270, 271],
					"name": "OnGridRunTime",
					"realname": "OnGrid RunTime",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [272, 273, 274, 275],
					"name": "OffGridRunTime",
					"realname": "OffGrid RunTime",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "h"
				}
			],
			"id": [
					{
					"position": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13],
					"name": "InverterSN",
					"realname": "Inverter SerialNumber",
					"datatype": "string"
				},
				{
					"position": [14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27],
					"name": "FactoryName",
					"realname": "Factory Name",
					"datatype": "string"
				},
				{
					"position": [28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41],
					"name": "ModuleName",
					"realname": "Module Name",
					"datatype": "string"
				}
			]
		}
	},
	"Solax-X3": {
		"config": {
			"RequestLiveData": ["#ClientID", "0x04", "0x00", "0x00", "0x00", "0x23"],
			"RequestIdData": ["#ClientID", "0x03", "0x00", "0x00", "0x00", "0x14"],
			"LiveDataFunctionCode": "0x04",
			"IdDataFunctionCode": "0x03",
			"LiveDataStartsAtPos": 3,
			"IdDataStartsAtPos": 3,
			"LiveDataErrorPos": 1,
			"LiveDataErrorCode": "0x83",
			"IdDataErrorPos": 1,
			"IdDataErrorCode": "0x84",
			"LiveDataSuccessPos": 1,
			"LiveDataSuccessCode": "0x03",
			"IdDataSuccessPos": 1,
			"IdDataSuccessCode": "0x04"
		},
		"data": {
			"livedata": [{
					"position": [0, 1],
					"name": "GridVoltage",
					"realname": "Grid Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [2, 3],
					"name": "GridCurrent",
					"realname": "Grid Current",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [4, 5],
					"name": "GridPower",
					"realname": "Grid Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [6, 7],
					"name": "PvVoltage1",
					"realname": "Pv Voltage 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [8, 9],
					"name": "PvVoltage2",
					"realname": "Pv Voltage 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [10, 11],
					"name": "PvCurrent1",
					"realname": "Pv Current 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [12, 13],
					"name": "PvCurrent2",
					"realname": "Pv Current 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [14, 15],
					"name": "GridFrequency",
					"realname": "Grid Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [16, 17],
					"name": "Temperature",
					"realname": "Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [20, 21],
					"name": "PowerDC1",
					"realname": "Power DC 1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [22, 23],
					"name": "PowerDC2",
					"realname": "Power DC 2",
					"datatype": "integer",
					"unit": "W"
				}
			],
			"id": [
					{
					"position": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13],
					"name": "InverterSN",
					"realname": "Inverter SerialNumber",
					"datatype": "string"
				},
				{
					"position": [14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27],
					"name": "FactoryName",
					"realname": "Factory Name",
					"datatype": "string"
				},
				{
					"position": [28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41],
					"name": "ModuleName",
					"realname": "Module Name",
					"datatype": "string"
				}
			]
		}
	}
}
)=====";

#endif
