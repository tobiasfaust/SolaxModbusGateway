// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  // Initiale Verbindung aufbauen
  connectWebSocket();

  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (ws && ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      GetInitData();
    }
  }, 100);
}

//var myInterval = setInterval(RefreshLiveData, 5000);

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "status";
  data['cmd']['callbackFn'] = "MyCallback";
  
  requestData(JSON.stringify(data));  
  RefreshLiveData();
}

// ************************************************
function RefreshLiveData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetItemsAsStream";
  //data['cmd']['subaction'] = "onlyactive";
  //data['cmd']['highlight'] = "true";
  
  requestData(JSON.stringify(data));
}

// ************************************************
function DoReboot() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "reboot";
  data['cmd']['callbackFn'] = "CallRebootPage";
  requestData(JSON.stringify(data));
}

// ************************************************
function DoReset() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "reset";
  data['cmd']['callbackFn'] = "CallRebootPage";
  requestData(JSON.stringify(data));
}

// ************************************************
function DoWifiReset() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "wifireset";
  data['cmd']['callbackFn'] = "CallRebootPage";
  requestData(JSON.stringify(data));
}

// ************************************************
function CallRebootPage() {
  window.location.href = "reboot.html";
}

// ************************************************
function MyCallback() {
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************