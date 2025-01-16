import * as global from './Javascript.js';

// ************************************************
window.addEventListener('load', init, false);
function init() {
  GetInitData();
}

// ************************************************
function GetInitData() {
  var data = {};
  data['action'] = "GetInitData";
  data['subaction'] = "navi";
  global.requestData(JSON.stringify(data));
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