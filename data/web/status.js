// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
  
  // Initiale Verbindung aufbauen
  connectWebSocket();
}

//var myInterval = setInterval(RefreshLiveData, 5000);

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "status";
  data['cmd']['callbackFn'] = "MyCallback";
  
  requestData(JSON.stringify(data));  
  RefreshLiveData();
}

// ************************************************
function RefreshLiveData() {
  var data = {};
  data['cmd']['action'] = "RefreshLiveData";
  data['cmd']['subaction'] = "onlyactive";
  data['cmd']['highlight'] = "true";
  
  requestData(JSON.stringify(data));
}

// ************************************************
function MyCallback() {
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************