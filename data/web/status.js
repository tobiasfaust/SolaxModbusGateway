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
  data['action'] = "GetInitData";
  data['subaction'] = "status";
  requestData(JSON.stringify(data), false, MyCallback);
  RefreshLiveData();
}

// ************************************************
function RefreshLiveData() {
  var data = {};
  data.action = "RefreshLiveData";
  data.subaction = "onlyactive";
  requestData(JSON.stringify(data), true);
}

// ************************************************
function MyCallback() {
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************