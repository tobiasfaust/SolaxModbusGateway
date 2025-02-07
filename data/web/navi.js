import * as global from './Javascript.js';

var currentVersion, newVersion = 0;

// ************************************************
export function init() {
  // Initiale Verbindung aufbauen
  global.connectWebSocket();
  checkUpdate();
  
  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (global.ws && global.ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      GetInitData();
    }
  }, 100);
}

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "navi";

  global.requestData(data);
}

// ************************************************
function checkUpdate() {
  // get deviceinfo the know where releases.json is
  fetch("/getdeviceinfo")
    .then(response => response.json())
    .then(data => {
      fetch("https://"+ data.owner + ".github.io/" + data.repository + "/firmware/releases.json")
        .then(response => response.json())
        .then(releases => {
          var newBuild = 0;
          for (let i = 0; i < releases.length; i++) {
            if (releases[i].stage == "stable" && releases[i].chipFamilies.includes(data.chipfamily) && releases[i].build > newBuild) {
              newBuild = releases[i].build;
              newVersion = releases[i].version + " (@" + releases[i].stage + ") / Build: " + newBuild;
            }
          }
          
          currentVersion = data.FWVersion;

          if (newBuild > data.build && newBuild > 0) {
            document.getElementById("updateInfoItem").classList.remove('hide');
          }

        });
    });
}

// ************************************************
export function showUpdateInfoInMainFrame() {
  document.getElementById("currentVersion").innerText = currentVersion;
  document.getElementById("newVersion").innerText = newVersion;
  const showUpdateInfo = document.getElementById("showUpdateInfo").cloneNode(true);
  if (top.frames['frame_main'].document.getElementById("showUpdateInfo")) {
    top.frames['frame_main'].document.getElementById("showUpdateInfo").classList.remove('hide');
  } else {
    top.frames['frame_main'].document.body.appendChild(showUpdateInfo);
    showUpdateInfo.classList.remove('hide');
  }
}

// ************************************************
export function highlightNavi(item) {
  const collection = document.getElementsByName('navi')
  
  for (let i = 0; i < collection.length; i++) {
    if (item.id == collection[i].id ) {
      document.getElementById(collection[i].id).classList.add('navi_active');
    } else {
      document.getElementById(collection[i].id).classList.remove('navi_active');
    }

  }

  top.frames["frame_main"].document.querySelector("#loader").style.visibility = "visible";  
  top.frames["frame_main"].document.querySelector("body").style.visibility = "hidden";
  
}

// ************************************************