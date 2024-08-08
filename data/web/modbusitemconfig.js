// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

// ************************************************
var myInterval = setInterval(RefreshLiveData, 5000);

// ************************************************
function GetInitData() {
  var data = {};
  data.action = "RefreshLiveData";
  data.subaction = "all";
  requestData(JSON.stringify(data), false, MyCallback);
}

// ************************************************
function MyCallback() {
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

function RefreshLiveData() {
  var data = {};
  data.action = "RefreshLiveData";
  data.subaction = "all";
  requestData(JSON.stringify(data), true);
}

// ************************************************
function ChangeActiveStatus(id) {
  obj = document.getElementById(id);
  //item = id.replace(/^activeswitch_(.*)$/g, "$1");
  var data = {};
  data.action = "SetActiveStatus";
  data.newState = (obj.checked?"true":"false");
  data.item = obj.name;
  requestData(JSON.stringify(data));
}