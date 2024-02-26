// ************************************************
window.addEventListener('load', init, false);
function init() {
  GetInitData();
  RefreshLiveData();
}

//var myInterval = setInterval(RefreshLiveData, 5000);

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
  requestData(JSON.stringify(data), true);
}

// ************************************************