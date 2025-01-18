import * as global from './Javascript.js';

// ************************************************
export function init1() {
  // erstelle ein Beispiel json mit Beispielwerten welches die funktion modbus::GetLiveDataAsJsonToWebserver generieren würde und weise das json der variable data zu. 
  
  var data = {"data": {"items": [
      {"name":  "InverterIdData", "realname": "InverterIdData", 
       "value": {"innerHTML": "1234", "data-id": "InverterIdData.value"}, 
       "active": {"checked": 1, "name": "InverterIdData"}, 
       "mqtttopic": "InverterIdData"
      },
      {"name":  "Power", "realname": "Power",
        "value": {"innerHTML": "1234", "data-id": "Power.value"},
        "active": {"checked": 0, "name": "Power"},
        "mqtttopic": "Power"
      }
    ]},
    "response": {"status": 1, "text": "successful"},
    "cmd": {
      "callbackFn": "mbitemconfig_Callback"
    }}

  global.handleJsonItems(data);
  datavalues = global.getFormData("DataForm");

  data = {"data-id": { "InverterIdData.value" : "684453556"},
  "response": {"status": 1, "text": "successful"},
  "cmd": {
    "highlight": "true"
  }}
  global.handleJsonItems(data);
}

// ************************************************
export function init() {
  // Initiale Verbindung aufbauen
  global.connectWebSocket();

  fetch('/getitems')
    .then(response => response.json())
    .then(data => {
      data['cmd'] = {};
      data['cmd']['callbackFn'] = "mbitemconfig_Callback";
      global.handleJsonItems(data);
    })
    .catch(error => console.error('Error fetching items:', error));

  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (global.ws && global.ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      RefreshLiveData();
    }
  }, 100);
}

// ************************************************
export const functionMap = {
  mbitemconfig_Callback: MyCallback
};

// ************************************************
function MyCallback(json) {
  global.transformCheckboxes()

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
function RefreshLiveData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetItemsAsStream";
  data['cmd']['highlight'] = "true";
  
  global.requestData(data);
}

// ************************************************
export function ChangeActiveStatus(id) {
  var obj = document.getElementById(id);
  
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "SetActiveStatus";
  data['cmd']['newState'] = (obj.checked?"true":"false");
  data['cmd']["item"] = obj.name;
  
  global.requestData(data);
}

// ************************************************
