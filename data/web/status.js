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

  var data = {
    "data": {
      "ipaddress": "192.168.1.1",
      "wifiname": "MyWiFi",
      "macaddress": "00:1A:2B:3C:4D:5E",
      "rssi": -50,
      "bssid": "00:1A:2B:3C:4D:5F",
      "mqtt_status": "Connected",
      "inverter_type": "Solax-TypeA",
      "inverter_serial": "SN123456789",
      "uptime": "24h 15m",
      "freeheapmem": 20480,
      "tr_webserial": {
        "className": "hide"
      }
    },
    "response": {
      "status": 1,
      "text": "successful"
    }
    , "cmd": {
      "action": "GetInitData",
      "subaction": "status"
      ,"callbackFn": "status_Callback"
      //, "highlight": "true"
    }
  };
  
  global.handleJsonItems(data);
}

export const functionMap = {
  status_Callback: MyCallback,
  status_CallRebootPage: CallRebootPage
};

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "status";
  data['cmd']['callbackFn'] = "status_Callback";
  
  global.requestData(data);  
}

// ************************************************
function MyCallback(json) {
  
  fetch('/getitems')
    .then(response => response.json())
    .then(data => {
        global.handleJsonItems(ReduceJsonOnlyActiveItems(data));
    })
    .catch(error => console.error('Error fetching items:', error));

  RefreshLiveData();
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************
function ReduceJsonOnlyActiveItems(json) {
  if (json.data && json.data.items) {
    json.data.items = json.data.items.filter(item => item.active && item.active.checked !== 0);
  }
  return json;
}

// ************************************************
function RefreshLiveData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetItemsAsStream";
  data['cmd']['subaction'] = "onlyactive";
  data['cmd']['highlight'] = "true";
  
  global.requestData(data);
}

// ************************************************
export function DoReboot() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "reboot";
  data['cmd']['callbackFn'] = "status_CallRebootPage";
  global.requestData(data);
}

// ************************************************
export function DoReset() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "reset";
  data['cmd']['callbackFn'] = "status_CallRebootPage";
  global.requestData(data);
}

// ************************************************
export function CallRebootPage(json) {
  window.location.href = "reboot.html";
}

// ************************************************
