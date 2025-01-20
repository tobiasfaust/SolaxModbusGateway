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

// ************************************************
function GetInitData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetInitData";
  data['cmd']['subaction'] = "navi";

  global.requestData(data);
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