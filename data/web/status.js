// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

var myInterval = setInterval(RefreshLiveData, 5000);

// ************************************************
function GetInitData() {
  var data = {};
  data['action'] = "GetInitData";
  data['subaction'] = "status";
  requestData(JSON.stringify(data), false);
}

// ************************************************
function RefreshLiveData() {
  var data = {};
  data.action = "RefreshLiveData";
  data.subaction = "onlyactive";
  requestData(JSON.stringify(data), true);
}

// ************************************************