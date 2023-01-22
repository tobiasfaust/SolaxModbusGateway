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
			"ClientIdPos": 0,
			"LiveDataFunctionCodePos": 1,
			"LiveDataFunctionCode": "0x04",
			"IdDataFunctionCodePos": 1,
			"IdDataFunctionCode": "0x03",
			"LiveDataStartsAtPos": 3,
			"IdDataStartsAtPos": 3,
			"LiveDataErrorPos": 1,
			"LiveDataErrorCode": "0x84",
			"IdDataErrorPos": 1,
			"IdDataErrorCode": "0x83",
			"LiveDataSuccessPos": 1,
			"LiveDataSuccessCode": "0x04",
			"IdDataSuccessPos": 1,
			"IdDataSuccessCode": "0x03"
		},
		"data": {
			"livedata": [{
					"position": [3, 4],
					"name": "GridVoltage",
					"realname": "Grid Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [5, 6],
					"name": "GridCurrent",
					"realname": "Grid Current",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [7, 8],
					"name": "GridPower",
					"realname": "Grid Power",
					"openwbtopic": "openWB/set/pv/W",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [9, 10],
					"name": "PvVoltage1",
					"realname": "Pv Voltage 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [11, 12],
					"name": "PvVoltage2",
					"realname": "Pv Voltage 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [13, 14],
					"name": "PvCurrent1",
					"realname": "Pv Current 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [15, 16],
					"name": "PvCurrent2",
					"realname": "Pv Current 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [17, 18],
					"name": "GridFrequency",
					"realname": "Grid Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [19, 20],
					"name": "Temperature",
					"realname": "Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [23, 24],
					"name": "PowerPv1",
					"realname": "Power PV 1",
					"openwbtopic": "openWB/set/pv/1/W",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [25, 26],
					"name": "PowerPv2",
					"realname": "Power PV 2",
					"openwbtopic": "openWB/set/pv/2/W",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [43, 44],
					"name": "BatVoltage",
					"realname": "Battery Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [45, 45],
					"name": "BatCurrent",
					"realname": "Battery Current",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [47, 48],
					"name": "BatPower",
					"realname": "Battery Power",
					"openwbtopic": "openWB/set/houseBattery/W",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [51, 52],
					"name": "BatTemp",
					"realname": "Battery Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [56, 57],
					"name": "BatCapacity",
					"realname": "Battery Capacity",
					"openwbtopic": "openWB/set/houseBattery/%Soc",
					"datatype": "integer",
					"unit": "%"
				},
				{
					"position": [58, 59, 60, 61],
					"name": "OutputEnergyCharge",
					"realname": "Output Energy Charge",
					"openwbtopic": "openWB/set/houseBattery/WhExported",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [64, 65],
					"name": "OutputEnergyChargeToday",
					"realname": "Output Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [66, 67, 68, 69],
					"name": "InputEnergyCharge",
					"realname": "Input Energy Charge",
					"openwbtopic": "openWB/set/houseBattery/WhImported",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [70, 71],
					"name": "InputEnergyChargeToday",
					"realname": "Input Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [141, 142, 143, 144],
					"name": "feedinPower",
					"realname": "FeedIn Energy Power to Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [145, 146, 147, 148],
					"name": "feedinEnergyTotal",
					"realname": "FeedIn Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [149, 150, 151, 152],
					"name": "consumedEnergyTotal",
					"realname": "Consumed Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [163, 164],
					"name": "EtodayToGrid",
					"realname": "Today Energy to Grid",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [165, 166, 167, 168],
					"name": "EnergyTotalToGrid",
					"realname": "Total Energy to Grid",
					"openwbtopic": "openWB/set/pv/WhCounter",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [276, 277, 278, 279],
					"name": "OnGridRunTime",
					"realname": "OnGrid RunTime",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [280, 281, 282, 283],
					"name": "OffGridRunTime",
					"realname": "OffGrid RunTime",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "h"
				}
			],
			"id": [
					{
					"position": [3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
					"name": "InverterSN",
					"realname": "Inverter SerialNumber",
					"datatype": "string"
				},
				{
					"position": [17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30],
					"name": "FactoryName",
					"realname": "Factory Name",
					"datatype": "string"
				},
				{
					"position": [31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42],
					"name": "ModuleName",
					"realname": "Module Name",
					"datatype": "string"
				}
			]
		},
		"set": [
			{
				"name": "TargetBatSOC",
				"request" : ["#ClientID", "0x06", "0x00", "0x83"]
			}
		]
	},
	"Solax-X3": {
		"config": {
			"RequestLiveData": [
				["#ClientID", "0x04", "0x00", "0x00", "0x00", "0x77"],
				["#ClientID", "0x04", "0x00", "0x78", "0x00", "0x77"]
			],
			"RequestIdData": ["#ClientID", "0x03", "0x00", "0x00", "0x00", "0x14"],
			"ClientIdPos": 0,
			"LiveDataFunctionCodePos": 1,
			"LiveDataFunctionCode": "0x04",
			"IDDataFunctionCodePos": 1,
			"IdDataFunctionCode": "0x03",
			"LiveDataStartsAtPos": 3,
			"IdDataStartsAtPos": 3,
			"LiveDataErrorPos": 1,
			"LiveDataErrorCode": "0x84",
			"IdDataErrorPos": 1,
			"IdDataErrorCode": "0x83",
			"LiveDataSuccessPos": 1,
			"LiveDataSuccessCode": "0x04",
			"IdDataSuccessPos": 1,
			"IdDataSuccessCode": "0x03"
		},
		"data": {
			"livedata": [
				{
					"position": [215, 216],
					"name": "GridVoltage_R",
					"realname": "Grid Voltage L1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [217, 218],
					"name": "GridCurrent_R",
					"realname": "Grid Current L1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [219, 220],
					"name": "GridPower_R",
					"realname": "Grid Power L1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [221, 222],
					"name": "GridFrequency_R",
					"realname": "Grid Frequency L1",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [223, 224],
					"name": "GridVoltage_S",
					"realname": "Grid Voltage L2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [225, 226],
					"name": "GridCurrent_S",
					"realname": "Grid Current L2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [227, 228],
					"name": "GridPower_S",
					"realname": "Grid Power L2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [229, 230],
					"name": "GridFrequency_S",
					"realname": "Grid Frequency L2",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [231, 232],
					"name": "GridVoltage_T",
					"realname": "Grid Voltage L3",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [233, 234],
					"name": "GridCurrent_T",
					"realname": "Grid Current L3",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [235, 236],
					"name": "GridPower_S",
					"realname": "Grid Power L3",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [237, 238],
					"name": "GridFrequency_T",
					"realname": "Grid Frequency L3",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [9, 10],
					"name": "PvVoltage1",
					"realname": "Pv Voltage 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [11, 12],
					"name": "PvVoltage2",
					"realname": "Pv Voltage 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [13, 14],
					"name": "PvCurrent1",
					"realname": "Pv Current 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [15, 16],
					"name": "PvCurrent2",
					"realname": "Pv Current 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [19, 20],
					"name": "Temperature",
					"realname": "Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [23, 24],
					"name": "PowerDC1",
					"realname": "Power DC 1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [25, 26],
					"name": "PowerDC2",
					"realname": "Power DC 2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [43, 44],
					"name": "BatVoltage",
					"realname": "Battery Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [45, 46],
					"name": "BatCurrent",
					"realname": "Battery Current",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [47, 48],
					"name": "BatPower",
					"realname": "Battery Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [51, 52],
					"name": "BatTemp",
					"realname": "Battery Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [59, 60],
					"name": "BatCapacity",
					"realname": "Battery Capacity",
					"datatype": "integer",
					"unit": "%"
				},
				{
					"position": [61, 62, 63, 64],
					"name": "OutputEnergyCharge",
					"realname": "Output Energy Charge",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [67, 68],
					"name": "OutputEnergyChargeToday",
					"realname": "Output Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [69, 70, 71, 72],
					"name": "InputEnergyCharge",
					"realname": "Input Energy Charge",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [73, 74],
					"name": "InputEnergyChargeToday",
					"realname": "Input Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [143, 144, 145, 146],
					"name": "feedinPower",
					"realname": "FeedIn Energy Power to Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [147, 148, 149, 150],
					"name": "feedinEnergyTotal",
					"realname": "FeedIn Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [151, 152, 153, 154],
					"name": "consumedEnergyTotal",
					"realname": "Consumed Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [163, 164],
					"name": "EnergyTodayToGrid",
					"realname": "Today Energy to Grid",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [167, 168, 169, 170],
					"name": "EnergyTotalToGrid",
					"realname": "Total Energy to Grid",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [280, 281, 282, 283],
					"name": "OnGridRunTime",
					"realname": "OnGrid RunTime",
					"datatype": "float",
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [284, 285, 286, 287],
					"name": "OffGridRunTime",
					"realname": "OffGrid RunTime",
					"datatype": "float",
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [239, 240],
					"name": "OffGridVoltage_R",
					"realname": "Off Grid Voltage L1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [241, 242],
					"name": "OffGridCurrent_R",
					"realname": "Off Grid Current L1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [248, 249],
					"name": "OffGridPowerActive_R",
					"realname": "Off Grid Power L1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [252, 253],
					"name": "OffGridVoltage_S",
					"realname": "Off Grid Voltage L2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [254, 255],
					"name": "OffGridCurrent_S",
					"realname": "Off Grid Current L2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [256, 257],
					"name": "OffGridPowerActive_S",
					"realname": "Off Grid Power L2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [260, 261],
					"name": "OffGridVoltage_T",
					"realname": "Off Grid Voltage L3",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [262, 263],
					"name": "OffGridCurrent_T",
					"realname": "Off Grid Current L3",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [264, 265],
					"name": "OffGridPowerActive_T",
					"realname": "Off Grid Power L3",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [268, 269, 270, 271],
					"name": "FeedInPowerPhase_R",
					"realname": "FeedIn Power Phase L1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [272, 273, 274, 275],
					"name": "FeedInPowerPhase_S",
					"realname": "FeedIn Power Phase L2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [276, 277, 278, 279],
					"name": "FeedInPowerPhase_T",
					"realname": "FeedIn Power Phase L3",
					"datatype": "integer",
					"unit": "W"
				}
			],
			"id": [
					{
					"position": [3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
					"name": "InverterSN",
					"realname": "Inverter SerialNumber",
					"datatype": "string"
				},
				{
					"position": [17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30],
					"name": "FactoryName",
					"realname": "Factory Name",
					"datatype": "string"
				},
				{
					"position": [31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42],
					"name": "ModuleName",
					"realname": "Module Name",
					"datatype": "string"
				}
			]
		}
	},
	"Growatt-SPH": {
		"config": {
			"RequestLiveData": [
				["#ClientID", "0x04", "0x00", "0x00", "0x00", "0x77"],
				["#ClientID", "0x04", "0x03", "0xE8", "0x00", "0x77"]
			],
			"RequestIdData": ["#ClientID", "0x03", "0x00", "0x00", "0x00", "0x14"],
			"ClientIdPos": 0,
			"LiveDataFunctionCodePos": 1,
			"LiveDataFunctionCode": "0x04",
			"IdDataFunctionCodePos": 1,
			"IdDataFunctionCode": "0x03",
			"LiveDataStartsAtPos": 3,
			"IdDataStartsAtPos": 3,
			"LiveDataErrorPos": 1,
			"LiveDataErrorCode": "0x84",
			"IdDataErrorPos": 1,
			"IdDataErrorCode": "0x83",
			"LiveDataSuccessPos": 1,
			"LiveDataSuccessCode": "0x04",
			"IdDataSuccessPos": 1,
			"IdDataSuccessCode": "0x03"
		},
		"data": {
			"livedata": [{
					"position": [185, 186, 187, 188],
					"name": "TotalEnergyPv2",
					"realname": "Erzeugte Energie Pv2",
					"openwbtopic": "openWB/set/pv/2/WhCounter",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [5, 6, 7, 8],
					"name": "PowerPv2",
					"realname": "Erzeugungsleistung Pv2",
					"openwbtopic": "openWB/set/pv/2/W",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [272, 273],
					"name": "BatVoltage",
					"realname": "Battery Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [77, 78],
					"name": "GridFrequency",
					"realname": "Grid Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [268, 269, 270, 271],
					"position2": [264, 265, 266, 267],
					"name": "BatChargingPower",
					"realname": "Battery Charging Power",
					"openwbtopic": "openWB/set/houseBattery/W",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [274, 275],
					"name": "BatCapacity",
					"realname": "Battery Capacity (SOC)",
					"openwbtopic": "openWB/set/houseBattery/%Soc",
					"datatype": "integer",
					"unit": "%"
				},
				{
					"position": [362, 363, 364, 365],
					"name": "InputEnergyCharge",
					"realname": "Geladene Energie Speicher (Wh)",
					"openwbtopic": "openWB/set/houseBattery/WhImported",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [354, 355, 356, 357],
					"name": "OutputEnergyCharge",
					"realname": "Entladene Energie Speicher (Wh)",
					"openwbtopic": "openWB/set/houseBattery/WhExported",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				}
			],
			"id": [{
				"position": [46, 47, 48, 49, 50, 51, 52, 53, 54, 55],
				"name": "InverterSN",
				"realname": "Inverter SerialNumber",
				"datatype": "string"
			}]
		}
	}
}
)=====";

#endif
