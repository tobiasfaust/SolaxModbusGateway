import * as global from './Javascript.js';

// ************************************************
export function init() {
  // Initiale Verbindung aufbauen
  global.connectWebSocket();

  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (global.ws && global.ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      GetInitData();
    }
  }, 100);
}

export function init1() {
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

  datavalues = global.getFormData("DataForm");
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
function MyCallback(json) {
  global.transformCheckboxes();
  global.handleRadioSelections();
  global.CreateSelectionListFromInputField('input[type=number][id^=GpioPin]', [global.gpio]);

  document.querySelectorAll('#DataForm input:not([type=checkbox]):not([type=radio]), #DataForm select').forEach(element => {
    element.addEventListener('blur', global.showMustSaveDialog);
  });
  
  document.querySelectorAll('#DataForm input[type=checkbox], #DataForm input[type=radio]').forEach(element => {
    element.addEventListener('click', global.showMustSaveDialog);
  });
  
  global.initDataValues();
  

  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************
