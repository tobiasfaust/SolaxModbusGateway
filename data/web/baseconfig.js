// ************************************************
window.addEventListener('DOMContentLoaded', init, false);
function init() {
  GetInitData();
}

// ************************************************
function GetInitData() {
  var data = {};
  data.action = "GetInitData";
  data.subaction = "baseconfig";
  requestData(JSON.stringify(data), true, MyCallback);
}

// ************************************************
function MyCallback() {
  CreateSelectionListFromInputField('input[type=number][id^=GpioPin]', [gpio]);
  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************