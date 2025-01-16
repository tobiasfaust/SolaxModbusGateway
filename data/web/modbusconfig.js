import * as global from './Javascript.js';

// ************************************************
export function init1() {
  // Initiale Verbindung aufbauen
  connectWebSocket();

  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (global.ws && global.ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      GetInitData();
    }
  }, 100);
}

export function init() {
  // erstelle ein Beispiel json mit Beispielwerten welches die funktion modbus::GetInitData generieren würde und weise das json der variable data zu. 
  var data = {
    "data": {
      "pin_rx": 16,
      "pin_tx": 17,
      "pin_rts": 5,
      "clientid": 1,
      "baudrate": 19200,
      "txintervallive": 10,
      "txintervalid": 60,
      "pin_RELAY1": 18,
      "pin_RELAY2": 19,
      "openwbversion": "1.2.3",
      "openwbmodulid": 1,
      "openwbbatteryid": 2,
      "inverters": [ [ { "inverter": {"value": "Kostal", "text": "Kostal"}}]],
    },
    "response": {
      "status": 1,
      "text": "successful"
    },
    "cmd": {
      "action": "GetInitData",
      "subaction": "status",
      "callbackFn": "mbconfig_Callback"
    }
  }
  
  global.handleJsonItems(data);
}

export const functionMap = {
  mbconfig_Callback: MyCallback
};

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "modbusconfig";
  data['cmd']['callbackFn'] = "mbconfig_Callback";
    
  global.requestData(data);  
}

// ************************************************
function MyCallback() {
  global.transformCheckboxes();
  global.handleRadioSelections();
  global.CreateSelectionListFromInputField('input[type=number][id^=GpioPin]', [global.gpio]);
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************