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
					"position": [21, 22],
					"name": "InverterStatus",
					"realname": "Inverter Status",
					"datatype": "integer",
					"mapping": [[0,"WaitMode"],[1,"CheckMode"],[2,"NormalMode"],[3, "FaultMode"],[4,"PermanentFaultMode"],[5,"UpdateMode"],[6,"EPSCheckMode"],[7,"EPSMode"],[8,"SelfTest"],[9,"IdleMode"]]
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
					"openwbtopic": "openWB/set/houseBattery/W",
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
					"openwbtopic": "openWB/set/houseBattery/%Soc",
					"unit": "%"
				},
				{
					"position": [58, 59, 60, 61],
					"name": "OutputEnergyChargeWh",
					"realname": "Output Energy Charge (Wh)",
					"openwbtopic": "openWB/set/houseBattery/WhExported",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [58, 59, 60, 61],
					"name": "OutputEnergyChargeKWh",
					"realname": "Output Energy Charge (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [64, 65],
					"name": "OutputEnergyChargeToday",
					"realname": "Output Energy Charge Today (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [66, 67, 68, 69],
					"name": "InputEnergyChargeWh",
					"realname": "Input Energy Charge (Wh)",
					"openwbtopic": "openWB/set/houseBattery/WhImported",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [66, 67, 68, 69],
					"name": "InputEnergyChargeKWh",
					"realname": "Input Energy Charge (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [70, 71],
					"name": "InputEnergyChargeToday",
					"realname": "Input Energy Charge Today (KWh)",
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
					"name": "EnergyTotalToGridWh",
					"realname": "Total Energy to Grid in Wh",
					"openwbtopic": "openWB/set/pv/WhCounter",
					"datatype": "integer", 
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [165, 166, 167, 168],
					"name": "EnergyTotalToGridKwh",
					"realname": "Total Energy to Grid in KWh",
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
					"name": "GridPower_T",
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
					"position": [21, 22],
					"name": "InverterStatus",
					"realname": "Inverter Status",
					"datatype": "integer",
					"mapping": [[0,"WaitMode"],[1,"CheckMode"],[2,"NormalMode"],[3, "FaultMode"],[4,"PermanentFaultMode"],[5,"UpdateMode"],[6,"EPSCheckMode"],[7,"EPSMode"],[8,"SelfTest"],[9,"IdleMode"]]
				},
				{
					"position": [23, 24],
					"name": "PowerPv1",
					"realname": "Power PV 1",
					"datatype": "integer",
					"openwbtopic": "openWB/set/pv/1/W",
					"unit": "W"
				},
				{
					"position": [25, 26],
					"name": "PowerPv2",
					"realname": "Power PV 2",
					"datatype": "integer",
					"openwbtopic": "openWB/set/pv/2/W",
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
					"openwbtopic": "openWB/set/houseBattery/W",
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
					"openwbtopic": "openWB/set/houseBattery/%Soc",
					"unit": "%"
				},
				{
					"position": [63, 64, 61, 62],
					"name": "OutputEnergyChargeWh",
					"realname": "Output Energy Charge (Wh)",
					"datatype": "integer",
					"openwbtopic": "openWB/set/houseBattery/WhExported",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [63, 64, 61, 62],
					"name": "OutputEnergyChargeKWh",
					"realname": "Output Energy Charge (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [67, 68],
					"name": "OutputEnergyChargeToday",
					"realname": "Output Energy Charge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [71, 72, 69, 70],
					"name": "InputEnergyChargeWh",
					"realname": "Input Energy Charge (Wh)",
					"datatype": "integer",
					"openwbtopic": "openWB/set/houseBattery/WhImported",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [71, 72, 69, 70],
					"name": "InputEnergyChargeKWh",
					"realname": "Input Energy Charge (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
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
					"position": [145, 146, 143, 144],
					"name": "feedinPower",
					"realname": "FeedIn Energy Power to Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [149, 150, 147, 148],
					"name": "feedinEnergyTotal",
					"realname": "FeedIn Energy Total",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [153, 154, 151, 152],
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
					"position": [169, 170, 167, 168],
					"name": "EnergyTotalToGridKwh",
					"realname": "Total Energy to Grid in KWh",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [169, 170, 167, 168],
					"name": "EnergyTotalToGridWh",
					"realname": "Total Energy to Grid in Wh",
					"datatype": "integer",
					"openwbtopic": "openWB/set/pv/WhCounter",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [280, 281, 278, 279],
					"name": "OnGridRunTime",
					"realname": "OnGrid RunTime",
					"datatype": "float",
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [284, 285, 282, 283],
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
					"position": [268, 269, 266, 267],
					"name": "FeedInPowerPhase_R",
					"realname": "FeedIn Power Phase L1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [272, 273, 270, 271],
					"name": "FeedInPowerPhase_S",
					"realname": "FeedIn Power Phase L2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [276, 277, 274, 275],
					"name": "FeedInPowerPhase_T",
					"realname": "FeedIn Power Phase L3",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [294, 295],
					"name": "OffGridYieldToday",
					"realname": "OffGrid Yield Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [296, 297],
					"name": "EChargeToday",
					"realname": "ECharge Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [300, 301, 298, 299],
					"name": "EChargeTotal",
					"realname": "ECharge Total",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [304, 305, 302, 303],
					"name": "SolarEnergyTotal",
					"realname": "SolarEnergy Total",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [306, 307],
					"name": "SolarEnergyToday",
					"realname": "SolarEnergy Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [312, 313, 310, 311],
					"name": "EnergyFeedin",
					"realname": "EnergyFeedin Today",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [316, 317, 314, 315],
					"name": "EnergyConsum",
					"realname": "EnergyConsum Today",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
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
			"livedata": [
				{
					"position": [3, 4],
					"name": "InverterRunState",
					"realname": "Inverter Status",
					"datatype": "integer",
					"mapping": [[0,"WaitMode"],[1,"NormalMode"],[3,"FaultMode"]]
				},
				{
					"position": [5, 6, 7, 8],
					"name": "InputPower",
					"realname": "Erzeugungsleistung Gesamt",
					"openwbtopic": "openWB/set/pv/W",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [13, 14, 15, 16],
					"name": "PowerPv1",
					"realname": "Erzeugungsleistung Pv1",
					"openwbtopic": "openWB/set/pv/1/W",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
				},
				{
					"position": [21, 22, 23, 24],
					"name": "PowerPv2",
					"realname": "Erzeugungsleistung Pv2",
					"openwbtopic": "openWB/set/pv/2/W",
					"datatype": "float",
					"factor": 0.1,
					"unit": "W"
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
					"position": [125, 126, 127, 128],
					"name": "TotalEnergyWhPv1",
					"realname": "Erzeugte Energie Pv1 in Wh",
					"openwbtopic": "openWB/set/pv/1/WhCounter",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [125, 126, 127, 128],
					"name": "TotalEnergyKwhPv1",
					"realname": "Erzeugte Energie Pv1 in Kwh",
					"openwbtopic": "openWB/set/pv/1/WhCounter",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [133, 134, 135, 136],
					"name": "TotalEnergyWhPv2",
					"realname": "Erzeugte Energie Pv2 in Wh",
					"openwbtopic": "openWB/set/pv/2/WhCounter",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [133, 134, 135, 136],
					"name": "TotalEnergyKwhPv2",
					"realname": "Erzeugte Energie Pv2 in Kwh",
					"openwbtopic": "openWB/set/pv/2/WhCounter",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [183, 284],
					"name": "BatVoltage",
					"realname": "Battery Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
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
					"name": "InputEnergyChargeWh",
					"realname": "Geladene Energie Speicher (Wh)",
					"openwbtopic": "openWB/set/houseBattery/WhImported",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [362, 363, 364, 365],
					"name": "InputEnergyChargeKwh",
					"realname": "Geladene Energie Speicher (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [354, 355, 356, 357],
					"name": "OutputEnergyChargeWh",
					"realname": "Entladene Energie Speicher (Wh)",
					"openwbtopic": "openWB/set/houseBattery/WhExported",
					"datatype": "integer",
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [354, 355, 356, 357],
					"name": "OutputEnergyChargeKwh",
					"realname": "Entladene Energie Speicher (KWh)",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				}
			],
			"id": [
			{
				"position": [49, 50, 51, 52, 53, 54, 55, 56, 57, 58],
				"name": "InverterSN",
				"realname": "Inverter SerialNumber",
				"datatype": "string"
			}]
		}
	},
	"Solax-Mic": {
		"config": {
			"RequestLiveData": [
				["#ClientID", "0x04", "0x04", "0x00", "0x00", "0x40"]
			],
			"RequestIdData": ["#ClientID", "0x03", "0x03", "0x00", "0x00", "0x14"],
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
			"livedata": [
				{
					"position": [3, 4],
					"name": "PvVoltage1",
					"realname": "Pv Voltage 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [5, 6],
					"name": "PvVoltage2",
					"realname": "Pv Voltage 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [7, 8],
					"name": "PvCurrent1",
					"realname": "Pv Current 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [9, 10],
					"name": "PvCurrent2",
					"realname": "Pv Current 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [11, 12],
					"name": "GridVoltage_R",
					"realname": "Grid Voltage L1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [13, 14],
					"name": "GridVoltage_S",
					"realname": "Grid Voltage L2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [15, 16],
					"name": "GridVoltage_T",
					"realname": "Grid Voltage L3",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [17, 18],
					"name": "GridFrequency_R",
					"realname": "Grid Frequency L1",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [19, 20],
					"name": "GridFrequency_S",
					"realname": "Grid Frequency L2",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [21, 22],
					"name": "GridFrequency_T",
					"realname": "Grid Frequency L3",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [23, 24],
					"name": "GridCurrent_R",
					"realname": "Grid Current L1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [25, 26],
					"name": "GridCurrent_S",
					"realname": "Grid Current L2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [27, 28],
					"name": "GridCurrent_T",
					"realname": "Grid Current L3",
					"datatype": "float",
					"factor": 0.1,
					"unit": "A"
				},
				{
					"position": [29, 30],
					"name": "Temperature",
					"realname": "Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				},
				{
					"position": [31, 32],
					"name": "OutputPower",
					"realname": "Output Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [33, 34],
					"name": "InverterStatus",
					"realname": "Inverter Status",
					"datatype": "integer",
					"mapping": [[0,"WaitMode"],[1,"CheckMode"],[2,"NormalMode"],[3,"FaultMode"],[4,"PermanentFaultMode"]]
				},
				{
					"position": [35, 36],
					"name": "GridPower_R",
					"realname": "Grid Power L1",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [37, 38],
					"name": "GridPower_S",
					"realname": "Grid Power L2",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [39, 40],
					"name": "GridPower_T",
					"realname": "Grid Power L3",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [41, 42],
					"name": "PowerPvTotal",
					"realname": "Erzeugungsleistung PV Gesamt",
					"datatype": "float",
					"unit": "W"
				},
				{
					"position": [43, 44],
					"name": "PowerPv1",
					"realname": "Erzeugungsleistung Pv1",
					"openwbtopic": "openWB/set/pv/1/W",
					"datatype": "float",
					"unit": "W"
				},
				{
					"position": [45, 46],
					"name": "PowerPv2",
					"realname": "Erzeugungsleistung Pv2",
					"openwbtopic": "openWB/set/pv/2/W",
					"datatype": "float",
					"unit": "W"
				},
				{
					"position": [75, 76, 73, 74],
					"name": "EnergyTotalToGridWh",
					"realname": "Total Energy to Grid in Wh",
					"openwbtopic": "openWB/set/pv/WhCounter",
					"datatype": "integer", 
					"factor": 100,
					"unit": "Wh"
				},
				{
					"position": [75, 76, 73, 74],
					"name": "EnergyTotalToGridKwh",
					"realname": "Total Energy to Grid in KWh",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [79, 80, 77, 78],
					"name": "EtodayToGrid",
					"realname": "Today Energy to Grid",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kWh"
				},
				{
					"position": [121, 122, 119, 120],
					"name": "GridPower",
					"realname": "Grid Power",
					"openwbtopic": "openWB/set/pv/W",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [125, 126, 123, 124],
					"name": "feedinEnergy",
					"realname": "FeedIn Energy",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [129, 130, 127, 128],
					"name": "ConsumeInEnergy",
					"realname": "ConsumeIn Energy",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				}
			],
			"id": [
				{
					"position": [3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14],
					"name": "InverterSN",
					"realname": "Inverter SerialNumber",
					"datatype": "string"
					}
				]
		}
	},
	"Sofar-KTL Solarmax-SGA": {
		"config": {
			"RequestLiveData": [
				["#ClientID", "0x03", "0x00", "0x00", "0x00", "0x30"]
			],
			"RequestIdData": ["#ClientID", "0x04", "0x20", "0x00", "0x00", "0x0B"],
			"ClientIdPos": 0,
			"LiveDataFunctionCodePos": 1,
			"LiveDataFunctionCode": "0x03",
			"IdDataFunctionCodePos": 1,
			"IdDataFunctionCode": "0x04",
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
			"livedata": [
				{
					"position": [3, 4],
					"name": "InverterStatus",
					"realname": "Inverter Status",
					"datatype": "integer",
					"mapping": [[0,"WaitMode"],[1,"CheckMode"],[2,"NormalMode"],[3, "FaultMode"],[4,"PermanentFaultMode"],[5,"UpdateMode"],[6,"EPSCheckMode"],[7,"EPSMode"],[8,"SelfTest"],[9,"IdleMode"]]
				},
				{
					"position": [15, 16],
					"name": "PvVoltage1",
					"realname": "Pv Voltage 1",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [17, 18],
					"name": "PvCurrent1",
					"realname": "Pv Current 1",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [19, 20],
					"name": "PvVoltage2",
					"realname": "Pv Voltage 2",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [21, 22],
					"name": "PvCurrent2",
					"realname": "Pv Current 2",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [23, 24],
					"name": "PowerPv1",
					"realname": "Power PV 1",
					"openwbtopic": "openWB/set/pv/1/W",
					"datatype": "integer",
					"factor": 10,
					"unit": "W"
				},
				{
					"position": [25, 26],
					"name": "PowerPv2",
					"realname": "Power PV 2",
					"openwbtopic": "openWB/set/pv/2/W",
					"datatype": "integer",
					"factor": 10,
					"unit": "W"
				},
				{
					"position": [27,28],
					"name": "GridPower",
					"realname": "Grid Power",
					"openwbtopic": "openWB/set/pv/W",
					"datatype": "float",
					"factor": 10,
					"unit": "W"
				},
				{
					"position": [31, 32],
					"name": "GridFrequency",
					"realname": "Grid Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [33, 34],
					"name": "GridVoltage",
					"realname": "Grid Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [35, 36],
					"name": "GridCurrent",
					"realname": "Grid Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [45, 46, 47, 48],
					"name": "EnergyTotalToGridKwh",
					"realname": "Total Energy to Grid in KWh",
					"datatype": "float", 
					"factor": 1,
					"unit": "kWh"
				},
				{
					"position": [49, 50, 51, 52],
					"name": "OnGridRunTime",
					"realname": "OnGrid RunTime",
					"datatype": "float", 
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [53, 54],
					"name": "EtodayToGrid",
					"realname": "Today Energy to Grid",
					"datatype": "float",
					"factor": 0.01,
					"unit": "kWh"
				},
				{
					"position": [55, 56],
					"name": "GenTimetoday",
					"realname": "Gen Time Today",
					"datatype": "float",
					"factor": 1,
					"unit": "min"
				},
				{
					"position": [57, 58],
					"name": "Temperature",
					"realname": "Temperature",
					"datatype": "integer",
					"unit": "&deg;C"
				}
			],
			"id": [
				{
					"position": [5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18],
					"name": "InverterSN",
					"realname": "Inverter SerialNumber",
					"datatype": "string"
				},
				{
					"position": [19, 20, 21, 22],
					"name": "SWVersion",
					"realname": "Software Version",
					"datatype": "string"
				},
				{
					"position": [23, 24, 25, 26],
					"name": "HWversion",
					"realname": "Hardware Version",
					"datatype": "string"
				}
			]
		}	
	},
	"Deye SUN-xx-SG04LP3": {
		"config": {
			"RequestLiveData": [
				["#ClientID", "0x03", "0x01", "0xF4", "0x00", "0x77"],
				["#ClientID", "0x03", "0x02", "0x6B", "0x00", "0x41"]
			],
			"RequestIdData": ["#ClientID", "0x04", "0x00", "0x00", "0x00", "0x07"],
			"ClientIdPos": 0,
			"LiveDataFunctionCodePos": 1,
			"LiveDataFunctionCode": "0x03",
			"IdDataFunctionCodePos": 1,
			"IdDataFunctionCode": "0x04",
			"LiveDataStartsAtPos": 3,
			"IdDataStartsAtPos": 3,
			"LiveDataErrorPos": 1,
			"LiveDataErrorCode": "0x80",
			"IdDataErrorPos": 1,
			"IdDataErrorCode": "0x84",
			"LiveDataSuccessPos": 1,
			"LiveDataSuccessCode": "0x03",
			"IdDataSuccessPos": 1,
			"IdDataSuccessCode": "0x04"
		},
		"data": {
			"livedata": [
				{
					"position": [3, 4],
					"name": "RunState",
					"realname": "Run State",
					"datatype": "integer",
					"mapping": [[0,"StandBy"],[1,"Self Check"],[2,"Normal Mode"],[3, "Alarm"],[4,"Fault Mode"]]
				},
				{
					"position": [5, 6],
					"name": "ActivePowerGenerationOfToday ",
					"realname": "Active Power Generation of Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [7, 8],
					"name": "ReactivePowerGenerationOfToday ",
					"realname": "Reactive Power Generation of Today",
					"datatype": "float",
					"factor": 0.1,
					"unit": "kVarh"
				},
				{
					"position": [9, 10],
					"name": "GridConnectionTimeOfToday ",
					"realname": "Grid Connection Time of Today",
					"datatype": "integer",
					"unit": "s"
				},
				{
					"position": [13,14,11, 12],
					"name": "ActivePowerGenerationOfTotal ",
					"realname": "Active Power Generation of Total",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [17,18,15, 16],
					"name": "ReactivePowerGenerationOfTotal ",
					"realname": "Reactive Power Generation of Total",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [31, 32],
					"name": "TodayChargeBattery",
					"realname": "Today Charge Battery",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [33, 34],
					"name": "TodayDischargeBattery",
					"realname": "Today Discharge Battery",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [37, 38,35,36],
					"name": "TotalChargeOfTheBattery",
					"realname": "Total Charge of the battery",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [41,42,39, 40],
					"name": "TotalDischargeOfTheBattery",
					"realname": "Total Discharge of the battery",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [43,44],
					"name": "DayGridBuyPower",
					"realname": "Day Grid Buy Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [45,46],
					"name": "DayGridSellPower",
					"realname": "Day Grid Sell Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [49,50,47,48],
					"name": "TotalGridBuyPower",
					"realname": "Total Grid Buy Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [53,54,51,52],
					"name": "TotalGridSellPower",
					"realname": "Total Grid Sell Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [55,56],
					"name": "DayLoadPower",
					"realname": "Day Load Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [59,60,57,58],
					"name": "TotalLoadPower",
					"realname": "Total Load Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [61,62],
					"name": "DayPVPower",
					"realname": "Day PV Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [63,64],
					"name": "DayPV1Power",
					"realname": "Day PV-1 Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [65,66],
					"name": "DayPV2Power",
					"realname": "Day PV-2 Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [67,68],
					"name": "DayPV3Power",
					"realname": "Day PV-3 Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [69,70],
					"name": "DayPV4Power",
					"realname": "Day PV-4 Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [73,74,71,72],
					"name": "TotalPVPower",
					"realname": "Total PV Power",
					"datatype": "float",
					"factor": 0.1,
					"unit": "KWh"
				},
				{
					"position": [81,82],
					"name": "GeneratorWorkingHoursPerDay",
					"realname": "Generator Working Hours per Day",
					"datatype": "float",
					"factor": 0.1,
					"unit": "h"
				},
				{
					"position": [83, 84],
					"name": "DCTransformatorTemperature",
					"realname": "DC-Transformator Temperature",
					"datatype": "float",
					"factor": 0.1,
					"valueAdd": -100,
					"unit": "°C"
				},
				{
					"position": [85, 86],
					"name": "HeatSinkTemperature",
					"realname": "Heatsink Temperature",
					"datatype": "float",
					"factor": 0.1,
					"valueAdd": -100,
					"unit": "°C"
				},
				{
					"position": [175, 176],
					"name": "BatteryTemperature",
					"realname": "Battery Temperature",
					"datatype": "float",
					"factor": 0.1,
					"valueAdd": -100,
					"unit": "&deg;C"
				},
				{
					"position": [177, 178],
					"name": "BatteryVoltage",
					"realname": "Battery Voltage",
					"datatype": "float",
					"factor": 0.01,
					"unit": "V"
				},
				{
					"position": [179, 180],
					"name": "BatteryCapacity",
					"realname": "Battery Capacity",
					"datatype": "integer",
					"unit": "%"
				},
				{
					"position": [183, 184],
					"name": "BatteryOutputPower",
					"realname": "Battery Output Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [185, 186],
					"name": "BatteryOutputCurrent",
					"realname": "Battery Output Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [187, 188],
					"name": "CorrectedAH",
					"realname": "Corrected AH",
					"datatype": "integer",
					"unit": "AH"
				},
				{
					"position": [199, 200],
					"name": "GridPhaseAVoltage",
					"realname": "Grid Phase-A Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [201, 202],
					"name": "GridPhaseBVoltage",
					"realname": "Grid Phase-B Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [203, 204],
					"name": "GridPhaseCVoltage",
					"realname": "Grid Phase-C Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [205, 206],
					"name": "GridABVoltage",
					"realname": "Grid AB Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [207, 208],
					"name": "GridBCVoltage",
					"realname": "Grid BC Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [209, 210],
					"name": "GridCAVoltage",
					"realname": "Grid CA Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [211, 212],
					"name": "PhaseAPowerInnerGrid",
					"realname": "Phase-A Power Inner Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [213, 214],
					"name": "PhaseBPowerInnerGrid",
					"realname": "Phase-B Power Inner Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [215, 216],
					"name": "PhaseCPowerInnerGrid",
					"realname": "Phase-C Power Inner Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [217, 218],
					"name": "ActivePowerSideToSideOfGrid",
					"realname": "Active Power Side to Side of Grid",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [219, 220],
					"name": "GridSideInsideTotalApparentPower",
					"realname": "Grid Side Inside Total Apparent Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [221, 222],
					"name": "GridSideFrequency",
					"realname": "Grid Side Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [223, 224],
					"name": "GridSideInnerCurrentA",
					"realname": "Grid Side Inner-Current-A",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [225, 226],
					"name": "GridSideInnerCurrentB",
					"realname": "Grid Side Inner-Current-B",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [227, 228],
					"name": "GridSideInnerCurrentC",
					"realname": "Grid Side Inner-Current-C",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [229, 230],
					"name": "OutOfGridCurrentA",
					"realname": "Out of Grid Current-A",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [231, 232],
					"name": "OutOfGridCurrentB",
					"realname": "Out of Grid Current-B",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [233, 234],
					"name": "OutOfGridCurrentC",
					"realname": "Out of Grid Current-C",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [235, 236],
					"name": "OutOfGridPowerA",
					"realname": "Out of Grid Power-A",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [237, 238],
					"name": "OutOfGridPowerB",
					"realname": "Out of Grid Power-B",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [239, 240],
					"name": "OutOfGridPowerC",
					"realname": "Out of Grid Power-C",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [246, 247],
					"name": "OutOfGridTotalPower",
					"realname": "Out of Grid Total Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [248, 249],
					"name": "OutOfGridTotalApparentPower",
					"realname": "Out of Grid Total Apperant Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [250, 251],
					"name": "GridConnectedPowerFactorPF",
					"realname": "Grid Connected Power Factor (PF)",
					"datatype": "float",
					"factor": 1000,
					"unit": ""
				},
				{
					"position": [252, 253],
					"name": "GridSidePhaseAPower",
					"realname": "Grid Side Phase-A Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [254, 255],
					"name": "GridSidePhaseBPower",
					"realname": "Grid Side Phase-B Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [256, 257],
					"name": "GridSidePhaseCPower",
					"realname": "Grid Side Phase-C Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [258, 259],
					"name": "GridSideTotalPower",
					"realname": "Grid Side Total Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [262, 263],
					"name": "InverterOutputPhaseAVoltage",
					"realname": "Inverter Output Phase-A Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [264, 265],
					"name": "InverterOutputPhaseBVoltage",
					"realname": "Inverter Output Phase-B Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [266, 267],
					"name": "InverterOutputPhaseCVoltage",
					"realname": "Inverter Output Phase-C Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [268, 269],
					"name": "InverterOutputPhaseACurrent",
					"realname": "Inverter Output Phase-A Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [270, 271],
					"name": "InverterOutputPhaseBCurrent",
					"realname": "Inverter Output Phase-B Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [272, 273],
					"name": "InverterOutputPhaseCCurrent",
					"realname": "Inverter Output Phase-C Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [274, 275],
					"name": "InverterOutputPhaseAPower",
					"realname": "Inverter Output Phase-A Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [276, 277],
					"name": "InverterOutputPhaseBPower",
					"realname": "Inverter Output Phase-B Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [278, 279],
					"name": "InverterOutputPhaseCPower",
					"realname": "Inverter Output Phase-C Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [280, 281],
					"name": "InverterOutputTotalPower",
					"realname": "Inverter Output Total Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [282, 283],
					"name": "InverterOutputTotalApparentPower",
					"realname": "Inverter Output Total Apparent Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [284, 285],
					"name": "InverterFrequency",
					"realname": "Inverter Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [288, 289],
					"name": "UPSLoadPowerA",
					"realname": "UPS Load Power-A",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [290, 291],
					"name": "UPSLoadPowerB",
					"realname": "UPS Load Power-B",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [292, 293],
					"name": "UPSLoadPowerC",
					"realname": "UPS Load Power-C",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [294, 295],
					"name": "UPSLoadTotalPower",
					"realname": "UPS Load Total Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [296, 297],
					"name": "LoadVoltageA",
					"realname": "Load Voltage-A",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [298, 299],
					"name": "LoadVoltageB",
					"realname": "Load Voltage-B",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [300, 301],
					"name": "LoadVoltageC",
					"realname": "Load Voltage-C",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [302, 303],
					"name": "LoadCurrentA",
					"realname": "Load Current-A",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [304, 305],
					"name": "LoadCurrentB",
					"realname": "Load Current-B",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [306, 307],
					"name": "LoadCurrentC",
					"realname": "Load Current-C",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [308, 309],
					"name": "LoadPowerA",
					"realname": "Load Power-A",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [310, 311],
					"name": "LoadPowerB",
					"realname": "Load Power-B",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [312, 313],
					"name": "LoadPowerC",
					"realname": "Load Power-C",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [314, 315],
					"name": "LoadTotalPower",
					"realname": "Load Total Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [316, 317],
					"name": "LoadTotalApparentPower",
					"realname": "Load Total Apparent Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [318, 319],
					"name": "LoadFrequency",
					"realname": "Load Frequency",
					"datatype": "float",
					"factor": 0.01,
					"unit": "Hz"
				},
				{
					"position": [330, 331],
					"name": "GenVoltageA",
					"realname": "Gen Voltage-A",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [332, 333],
					"name": "GenVoltageB",
					"realname": "Gen Voltage-B",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [334, 335],
					"name": "GenVoltageC",
					"realname": "Gen Voltage-C",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [336, 337],
					"name": "GenPowerA",
					"realname": "Gen Power-A",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [338, 339],
					"name": "GenPowerB",
					"realname": "Gen Power-B",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [340, 341],
					"name": "GenPowerC",
					"realname": "Gen Power-C",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [342, 343],
					"name": "GenTotalPower",
					"realname": "Gen Total Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [352, 353],
					"name": "PV1InputPower",
					"realname": "PV1 Input Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [354, 355],
					"name": "PV2InputPower",
					"realname": "PV2 Input Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [356, 357],
					"name": "PV3InputPower",
					"realname": "PV3 Input Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [358, 359],
					"name": "PV4InputPower",
					"realname": "PV4 Input Power",
					"datatype": "integer",
					"unit": "W"
				},
				{
					"position": [360, 361],
					"name": "PV1DCVoltage",
					"realname": "PV1 DC Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [362, 363],
					"name": "PV1DCCurrent",
					"realname": "PV1 DC Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [364, 365],
					"name": "PV2DCVoltage",
					"realname": "PV2 DC Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [366, 367],
					"name": "PV2DCCurrent",
					"realname": "PV2 DC Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [368, 369],
					"name": "PV3DCVoltage",
					"realname": "PV3 DC Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [370, 371],
					"name": "PV3DCCurrent",
					"realname": "PV3 DC Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				},
				{
					"position": [372, 373],
					"name": "PV4DCVoltage",
					"realname": "PV4 DC Voltage",
					"datatype": "float",
					"factor": 0.1,
					"unit": "V"
				},
				{
					"position": [374, 375],
					"name": "PV4DCCurrent",
					"realname": "PV4 DC Current",
					"datatype": "float",
					"factor": 0.01,
					"unit": "A"
				}
			],
			"id": [
				{
					"position": [9,10,11,12,13,14,15,16],
					"name": "SN",
					"realname": "SerialNumber",
					"datatype": "string"
				}
			]
		}
	}
}
)=====";

#endif
