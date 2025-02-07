import * as global from './Javascript.js';

// ************************************************
export function init1() {
  
  var data = {"data": {"items": [
      {"name":  "InverterIdData", "realname": "InverterIdData", 
       "value": {"innerHTML": "1234", "data-id": "InverterIdData.value"}, 
       "active": {"checked": 1, "name": "InverterIdData"}, 
       "mqtttopic": "InverterIdData"
      },
      {"name":  "Power", "realname": "Power",
        "value": {"innerHTML": "1234", "data-id": "Power.value"},
        "active": {"checked": 0, "name": "Power"},
        "mqtttopic": "Power"
      }
    ]},
    "response": {"status": 1, "text": "successful"},
    "cmd": {
      "callbackFn": "mbitemconfig_ItemCallback"
    }}

  global.handleJsonItems(data);


  data = {"globalEnabled": "1",  "data": {"setitems": [{"name": "setUnlockSettings","realname": "Unlock Settings","active": {"checked": 0, "name": "setUnlockSettings"},"info": "send the 4 digit advanced password","subscription": "home/Solax/set/setUnlockSettings"},{"name": "setTargetBatSOC","realname": "Target SoC","active": {"checked": 0, "name": "setTargetBatSOC"},"info": "set battery SOC: 0 - 100 in percent","subscription": "home/Solax/set/setTargetBatSOC"},{"name": "setOperationMode","realname": "Operation Mode","active": {"checked": 0, "name": "setOperationMode"},"info": "setting of 6 possible operation modes","subscription": "home/Solax/set/setOperationMode","mapping": {"data-mapping": "[['SelfUse',0],['FeedInPriority',1],['BackupMode',2],['ManuelMode',3],['PeakShaving',4],['TUOMode',5]]"}} ]}, 
  "object_id": "home/Solax",
    "cmd": {
      "callbackFn": "mbitemconfig_SetterCallback"
    }} 
  global.handleJsonItems(data);


  data = {"data-id": { "InverterIdData.value" : "684453556"},
  "response": {"status": 1, "text": "successful"},
  "cmd": {
    "highlight": "true"
  }}
  global.handleJsonItems(data);
}

// ************************************************
export function init() {
  // Initiale Verbindung aufbauen
  global.connectWebSocket();

  fetch('/getitems')
    .then(response => response.json())
    .then(data => {
      data['cmd'] = {};
      data['cmd']['callbackFn'] = "mbitemconfig_ItemCallback";
      global.handleJsonItems(data);
    })
    .catch(error => console.error('Error fetching items:', error));

  fetch('/getsetter')
    .then(response => response.json())
    .then(data => {
      data['cmd'] = {};
      data['cmd']['callbackFn'] = "mbitemconfig_SetterCallback";
      global.handleJsonItems(data);
    })
    .catch(error => console.error('Error fetching setters:', error));

  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (global.ws && global.ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      RefreshLiveData();
    }
  }, 100);
}

// ************************************************
export const functionMap = {
  mbitemconfig_ItemCallback: MyItemCallback,
  mbitemconfig_SetterCallback: MySetterCallback
};

// ************************************************
function MyItemCallback(json) {
  global.transformCheckboxes();
  
  document.querySelectorAll('#DataForm input:not([type=checkbox]):not([type=radio]), #DataForm select').forEach(element => {
    element.addEventListener('blur', global.showMustSaveDialog);
  });
    
  document.querySelectorAll('#DataForm input[type=checkbox], #DataForm input[type=radio]').forEach(element => {
    element.addEventListener('click', global.showMustSaveDialog);
  });
    
  global.initDataValues();

  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************
function MySetterCallback(json) {
  MyItemCallback(json);
    
  if ("data" in json && "setitems" in json["data"] && json["data"]["setitems"].length == 0) {
    document.getElementById("setterNa").classList.remove("hide");
  } 
  else if ("globalEnabled" in json && json["globalEnabled"] == 0) {
    document.getElementById("setterDeactive").classList.remove("hide");
  }
  else {
    document.getElementById("settable").classList.remove("hide");
  }

  // handle data-mapping and add them to topic as tooltip
  document.querySelectorAll('[data-mapping]').forEach(element => {
    try {
      const mapping = JSON.parse(element.getAttribute('data-mapping').replace(/'/g, '"'));
      const obj = document.getElementById(element.id);
      var info = "";
      
      for (var i = 0; i < mapping.length; i++) {
        if (info.length > 0) info += "<br>"; 
        info += "- " + mapping[i][0];
      }
      
      info = "possible values:<br><hr>" + info;
      createTooltip(obj, info);

    } catch (e) {
      console.error('Invalid JSON:', e);
    }
  });

  // handle data-info and add them to realname as tooltip
  document.querySelectorAll('[data-info]').forEach(element => {
    const info = element.getAttribute('data-info');
    const obj = document.getElementById(element.id);
      
    createTooltip(obj, info);
  });
}

// ************************************************
function createTooltip(obj, tooltip) {
  var dfn = document.createElement('dfn');
  dfn.classList.add('tooltip_simple');
  obj.parentNode.replaceChild(dfn, obj);
  dfn.appendChild(obj);

  var span = document.createElement('span');
  span.setAttribute('role', 'tooltip_simple');
  span.innerHTML = tooltip;
  dfn.appendChild(span);
}

// ************************************************
function RefreshLiveData() {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "GetItemsAsStream";
  data['cmd']['highlight'] = "true";
  
  global.requestData(data);
}
 
 
// ************************************************
export function ChangeActiveStatus(id) {
  var obj = document.getElementById(id);
  
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "SetActiveStatus";
  data['cmd']['newState'] = (obj.checked?"true":"false");
  data['cmd']["item"] = obj.name;
  
  global.requestData(data);
}

// ************************************************

export function setToggleIcon(targetTable, element) {
  if (element.classList.contains('fa-toggle-off')) {
    element.classList.remove('fa-toggle-off');
    element.classList.add('fa-toggle-on');
    setCheckboxes(targetTable, "true");
  } else {
    element.classList.remove('fa-toggle-on');
    element.classList.add('fa-toggle-off');
    setCheckboxes(targetTable, false);
  }
}

export function setCheckboxes(targetTable, state) {
  document.querySelectorAll(`#${targetTable} input[type=checkbox]`).forEach(checkbox => {
    if (typeof state == 'undefined') {
      //checkbox.checked = !checkbox.checked;
      checkbox.click();
    } else if ( typeof state !== 'undefined' && state && !checkbox.checked) {
      //checkbox.checked = true;
      checkbox.click();
    } else if (typeof state !== 'undefined' && !state && checkbox.checked) {
      //checkbox.checked = false;
      checkbox.click();
    }
  });
  global.showMustSaveDialog();
}