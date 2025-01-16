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
  var data = {
    "data": {
      "mqttroot": "exampleRoot",
      "mqttserver": "exampleServer",
      "mqttport": 1883,
      "mqttuser": "exampleUser",
      "mqttpass": "examplePass",
      "mqttbasepath": "exampleBasePath",
      "debuglevel": 2,
      "sel_wifi": 1,
      "sel_eth": 0,
      "useRandomClientID": 1,
      "sel_auth": 0,
      "auth_user": "authUser",
      "auth_pass": "authPass",
      "GpioPin_serial_rx": 15,
      "GpioPin_serial_tx": 16
    },
    "response": {
      "status": 1,
      "text": "successful"
    }
    , "cmd": {
      "action": "GetInitData",
      "subaction": "baseconfig"
      ,"callbackFn": "baseconfig_Callback"
      , "highlight": "true"
    }
  };
  
  global.handleJsonItems(data);
}

export const functionMap = {
  baseconfig_Callback: MyCallback
};

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "baseconfig";
  data['cmd']['callbackFn'] = "baseconfig_Callback";
      
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