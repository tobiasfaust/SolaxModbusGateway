/*****************************************************************************************
 * @file /data/web/Javascript.js
 * @description This file contains various JavaScript functions and constants used for handling GPIO configurations, 
 *              data fetching, applying JSON data to HTML templates, creating selection lists from input fields, 
 *              and transforming checkboxes into styled on/off switches. The functions are designed to work with 
 *              a web interface for managing and configuring GPIO ports and other related settings.
 * @version 1.0.0
 * @date 2023-10-01
 * 
 * @author Tobias.Faust
 * 
 * @license MIT
 * 
 *****************************************************************************************/

/*****************************************************************************************
 * Definition of constants
 *****************************************************************************************/

export const gpio_disabled = [];

export const gpio = [  {port: 1,  name:'D1/TX0'},
                {port: 2 , name:'D2'},
                {port: 3,  name:'D3/RX0'},
                {port: 4 , name:'D4'},
                {port: 5 , name:'D5'},
                {port: 6 , name:'D6'},
                {port: 7 , name:'D7'},
                {port: 8 , name:'D8'},
                {port: 9 , name:'D9/RX1'},
                {port: 10, name:'D10/TX1'},
                {port: 12, name:'D12'},
                {port: 13, name:'D13'},
                {port: 14, name:'D14'},
                {port: 15, name:'D15'},
                {port: 16, name:'D16/RX2'},
                {port: 17, name:'D17/TX2'},
                {port: 18, name:'D18'},
                {port: 19, name:'D19'},
                {port: 21, name:'D21/SDA'},
                {port: 22, name:'D22/SCL'},
                {port: 23, name:'D23'},
                {port: 25, name:'D25'},
                {port: 26, name:'D26'},
                {port: 27, name:'D27'},
                {port: 32, name:'D32'},
                {port: 33, name:'D33'},
                {port: 34, name:'D34'},
                {port: 35, name:'D35'},
                {port: 36, name:'D36'},
                {port: 39, name:'D39'}
              ];

export const gpioanalog = [  {port: 36, name:'ADC1_CH0 - GPIO36'},
                  {port: 37, name:'ADC1_CH1 - GPIO37'},
                  {port: 38, name:'ADC1_CH2 - GPIO38'},
                  {port: 39, name:'ADC1_CH3 - GPIO39'},
                  {port: 32, name:'ADC1_CH4 - GPIO32'},
                  {port: 33, name:'ADC1_CH5 - GPIO33'},
                  {port: 34, name:'ADC1_CH6 - GPIO34'},
                  {port: 35, name:'ADC1_CH7 - GPIO35'},
                  {port: 4 , name:'ADC2_CH0 - GPIO4'},
                  {port: 0 , name:'ADC2_CH1 - GPIO0'},
                  {port: 2 , name:'ADC2_CH2 - GPIO2'},
                  {port: 15, name:'ADC2_CH3 - GPIO15'},
                  {port: 13, name:'ADC2_CH4 - GPIO13'},
                  {port: 12, name:'ADC2_CH5 - GPIO12'},
                  {port: 14, name:'ADC2_CH6 - GPIO14'},
                  {port: 27, name:'ADC2_CH7 - GPIO27'},
                  {port: 25, name:'ADC2_CH8 - GPIO25'},
                  {port: 26, name:'ADC2_CH9 - GPIO26'}
               ];

import { functionMap as statusFunctionMap } from './status.js';
import { functionMap as baseconfigFunctionMap } from './baseconfig.js';
import { functionMap as mbconfigFunctionMap } from './modbusconfig.js';
import { functionMap as mbitemconfigFunctionMap } from './modbusitemconfig.js';
import { functionMap as rawdataFunctionMap } from './rawdata.js';
import { functionMap as filesFunctionMap } from './handlefiles.js';

const combinedFunctionMap = {
  ...statusFunctionMap,
  ...baseconfigFunctionMap,
  ...mbconfigFunctionMap,
  ...mbitemconfigFunctionMap,
  ...rawdataFunctionMap,
  ...filesFunctionMap
};

export let ws;    // websocket handle
var datavalues;   // form data values as string to check, if "needToSave" Dialog should be shown

var timer; // ID of setTimout Timer -> setResponse
let reconnectInterval = 5000; // 5 seconds interval to reconnect websocket connection

/******************************************************************************************
 * Connect to WebSocket server
 * *****************************************************************************************/
export function connectWebSocket() {
  window.addEventListener('beforeunload', function() {
    if (ws) {
      ws.close();
    }
  }, false);

  document.addEventListener('visibilitychange', function() {
    if (document.visibilityState === 'visible') {
      if (!ws || ws.readyState === WebSocket.CLOSED) {
        console.log('Reconnecting WebSocket due to visibility change');
        connectWebSocket();
      }
    } else {
      if (ws) {
        console.log('Closing WebSocket due to visibility change');
        ws.close();
      }
    }
  });

  if (document.visibilityState != 'visible') {
    console.log('Not connecting WebSocket due to visibility state:', document.visibilityState);
    return;
  }

  ws = new WebSocket(location.origin.replace(/^http/, 'ws') + '/ajaxws');
  //ws = new WebSocket('ws://10.0.2.150/ajaxws'); 
  var wsStatus = document.getElementById('ws-status');

  ws.onopen = function() {
    console.log('WebSocket connection opened');
    if (wsStatus) wsStatus.style.backgroundColor = 'green';
  };

  ws.onmessage = function(event) {
    //try {
      const json = JSON.parse(event.data);
      console.log('Received JSON:', json);
      handleJsonItems(json);
    //} catch (e) {
    //  console.error('Invalid JSON received:', event.data);
    //}
  };

  ws.onclose = function() {
    console.log('WebSocket connection closed, attempting to reconnect in ' + reconnectInterval / 1000 + ' seconds');
    if (wsStatus) wsStatus.style.backgroundColor = 'yellow';
    setTimeout(connectWebSocket, reconnectInterval);
  };

  ws.onerror = function(error) {
    console.error('WebSocket error:', error);
    if (wsStatus) wsStatus.style.backgroundColor = 'red';
    ws.close();
  };
}



/******************************************************************************************
 * activate all radioselections after pageload to hide unnecessary elements
 * Works for all checkbox and radio elements with onclick="radioselection(show, hide)"
 * 
 ******************************************************************************************/
export function handleRadioSelections() {
  var radios = document.querySelectorAll('input[type=radio][onclick*=radioselection]:checked');
  for (var i = 0; i < radios.length; i++) {
    if (radios[i].onclick) {
      var onclickStr = radios[i].getAttribute('onclick');
      var match = onclickStr.match(/radioselection\((.*)\)/);
      if (match) {
        eval("radioselection(" + match[1] + ")");
      }
    }
  }

  var checkboxes = document.querySelectorAll('input[type=checkbox][onclick*=onCheckboxSelection]');
  for (var i = 0; i < checkboxes.length; i++) {
    if (checkboxes[i].onclick) {
      var onclickStr = checkboxes[i].getAttribute('onclick');
      var match = onclickStr.match(/onCheckboxSelection\((.*)\)/);
      if (match) {
        eval("onCheckboxSelection(" + match[1] + ")");
      }
    }
  }
}

/*****************************************************************************************
 * central function to send data to server
 * @param {*} json -> json object to send
 * @param {*} highlight -> highlight on/off
 * @param {*} callbackFn -> callback function to call after data is fetched
 * @returns {*} void
******************************************************************************************/
export function requestData(json) {
  if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
    console.log('WebSocket is open, sending data:', json);
    ws.send(JSON.stringify(json));
  } else {
    console.log('WebSocket not open');
    setResponse(false, 'WebSocket not open, could not send data');
  }
}

/*****************************************************************************************
 * @description This function updated values according their data-id in DOM elements.
 * @param {*} json: JSON object containing the data-id values to update
 * @param {*} highlight: boolean value to highlight the updated elements
 * @returns {*} void
 * @example updateDataID({"data-id":{"InverterSN.value":"123456789"}}, true)
 * *****************************************************************************************/
function updateDataID(json, highlight) {
  if (json["data-id"]) {
    for (const key in json["data-id"]) {
      const elements = document.querySelectorAll(`[data-id="${key}"]`);
      elements.forEach(element => {
        element.innerHTML = json["data-id"][key];
        if (highlight && element.classList.contains('ajaxchange')) { 
        	element.classList.add('highlightOn');
          setTimeout(function() {document.getElementById(element.id).classList.remove('highlightOn')}, 1000);
        }
        
      });
    }
  }
}


/*****************************************************************************************
 *
 * definition of applying jsondata to html templates
 * @param {*} _obj -> object to apply key
 * @param {*} _key -> key to apply
 * @param {*} _val -> value to apply
 * @param {*} counter -> counter of array
 * @param {*} tplHierarchie -> hierarchie of template
 * @param {*} highlight -> highlight on/off
 * @returns {*} void
 *
******************************************************************************************/
function applyKey (_obj, _key, _val, counter, tplHierarchie, highlight) {
  if (_obj.id == _key || _obj.id == tplHierarchie +"."+ _key) {
	  if (['SPAN', 'DIV', 'TD', 'DFN'].includes(_obj.tagName)) {
      if (highlight && _obj.classList.contains('ajaxchange')) {
        _obj.classList.add('highlightOn');
        setTimeout(function() {document.getElementById(_obj.id).classList.remove('highlightOn')}, 1000);
      }  
      _obj.innerHTML = _val;

    } else if (_obj.tagName == 'INPUT' && ['checkbox','radio'].includes(_obj.type)) {
      if (_val == true) _obj.checked = true;
    } else if (_obj.tagName == 'OPTION') {
      if (_val == true) _obj.selected = true;
    } else {
      _obj.value = _val;
    }
  } else {
  	// using parent object 
    if (_key in _obj) {
      if (highlight && _obj.classList.contains('ajaxchange')) { 
        _obj.classList.add('highlightOn'); 
        setTimeout(function() {document.getElementById(_obj.id).classList.remove('highlightOn')}, 1000);
      }
      
      _obj[_key] = _val;
    } else {
      _obj.setAttribute(_key, _val);
    }
  }
}

/*****************************************************************************************
 * Apply a set of keys from an Array or an Object
 * @param {*}  json	-> der json teil der angewendet werden soll
 * @param {*}  _tpl	-> das documentFragment auf welches das json agewendet werden soll
 * @param {*}  ObjID -> ggf eine ID im _tpl auf die "key" und "value" des jsons agewendet werden soll
 *					wenn diese undefinied ist, ist die ID = json[key]
 * @param {*}  counter -> gesetzt, wenn innerhalb eines _tpl arrays die ID des Objektes hochgezählt wurde           
 * @param {*}  highlight -> wenn in einem objekt die klasse "ajaxchange" gesetzt ist, so wird die Klasse "highlightOn" angewendet
*****************************************************************************************/

function applyKeys(json, _tpl, ObjID, counter, tplHierarchie, highlight) {
  for (var key in json) {
    if (Array.isArray(json[key])) {
    	applyTemplate((json[key]), key, _tpl, tplHierarchie, highlight);
    } else if (typeof json[key] === 'object') {
      applyKeys(json[key], _tpl, key, counter, tplHierarchie, highlight);
    } else {
      try {
        var _obj, _objID
        if (ObjID) {
        	// obj is parent object, key ist jetzt eigenschaft, nicht ID
          if (ObjID + "_" + counter == _tpl.firstElementChild.id)  {
          	//die firstChildID ist schon das Object
            _objID = _tpl.firstElementChild.id;
          } else {
          	_objID = (tplHierarchie?tplHierarchie +".":"") + ObjID;
          }
        } else {
        	// key ist die ID
          _objID = (tplHierarchie?tplHierarchie +".":"") + key
        }
        
        if (document.getElementById(_objID)) {
          // exists already in DOM
          _obj = document.getElementById(_objID);
        } else if (_tpl.getElementById(_objID + "_" + counter)) {
        	
          _obj = _tpl.getElementById(_objID + "_" + counter);
        } else {
          _obj = _tpl.getElementById(_objID);
        }
        applyKey(_obj, key, json[key], counter, tplHierarchie, highlight);
      } catch(e) {}
    }
  }
}

/*****************************************************************************************
 * apply template to document
 * @param {*} TemplateJson: json object to apply
 * @param {*} templateID: id of template to apply
 * @param {*} doc: document to apply
 * @param {*} tplHierarchie: hierarchie of template
 * @param {*} highlight: highlight on/off
 * @returns {*} void
*****************************************************************************************/
function applyTemplate(TemplateJson, templateID, doc, tplHierarchie, highlight) {
  if (Array.isArray(TemplateJson)) {
    for (var i=0; i < TemplateJson.length; i++) {
      if(doc.getElementById(templateID)) {
        var _tpl = document.importNode(doc.getElementById(templateID).content, true);
        var _parentObj = doc.getElementById(templateID).parentNode;
        try {
          
          //adjust id of first element, often not included in querySelectorAll statement (example: <tr>)
          // firstchild.id contains hierarchie of templates to keep unique id´s 
          if (_tpl.firstElementChild.id) {
          	_tpl.firstElementChild.id = (tplHierarchie?tplHierarchie +".":"") + _tpl.firstElementChild.id + "_" + [i];
          } else { 
          	_tpl.firstElementChild.id = (tplHierarchie?tplHierarchie +".":"") + templateID + "_" + [i];
          }
          
          //adjust all id´s
          const o = _tpl.querySelectorAll("*");
          for (var j=0; j<o.length; j++) {
            if (o[j].id && o[j].id != _tpl.firstElementChild.id && o[j].tagName!="TEMPLATE") {
              o[j].id = _tpl.firstElementChild.id +"."+ o[j].id;
            }
          }
          
          
        } catch(e) {}

				applyKeys(TemplateJson[i], _tpl, undefined, i, (_tpl.firstElementChild.id), highlight);

        if (document.getElementById(_tpl.firstElementChild.id)) {
          // item already in DOM
        } else { 
          _parentObj.appendChild(_tpl);
        }
      }      
    }
  }
}

/*****************************************************************************************
 * apply Javascript variables to the window object from a JSON object
 * @param {*} json: JSON object containing the variables to apply
 * @returns {*} void
 * @example applyJS({"myVar": "myValue"})
 * *****************************************************************************************/
function applyJS(json) {
	for (var key in json) {
  	window[key] = json[key];
  }
}

/*****************************************************************************************
 * Main function to handle JSON response
 * @param {*} json: JSON object containing the response data
 * @returns {*} void
 * @example handleJsonItems({"data-id": {"InverterSN": "123456789"}, "cmd": {"action": "GetInitData", "subaction": "status", "callbackFn": "MyCallback"}, "
 *                          "response": {"status": 1, "text": "OK"}})
 * *****************************************************************************************/
export function handleJsonItems(json) {
  const callbackFn = (typeof json['cmd'] !== 'undefined' && typeof json['cmd']['callbackFn'] !== 'undefined') ? json['cmd']['callbackFn'] : undefined; 
  const highlight = (typeof json['cmd'] !== 'undefined' && typeof json['cmd']['highlight'] !== 'undefined') ? json['cmd']['highlight'] : false;

  if ("data" in json) {
    applyKeys(json.data, document, undefined, undefined, '', highlight);
  }

  if ('js' in json) { 
  	applyJS(json.js);
  }

  if ('response' in json) {
    try {
      if (json.response.status == 1) {setResponse(true, json.response.text);}
      if (json.response.status == 0) {setResponse(false, json.response.text);}
    } catch(e) {setResponse(false, 'unknow error');}
  }

  if ("data-id" in json) {
    updateDataID(json, highlight);
  }

	// DOM objects now ready
  if (callbackFn && typeof combinedFunctionMap[callbackFn] === 'function') {
    combinedFunctionMap[callbackFn](json);
  }
}

/*****************************************************************************************
 * show response
 * @param {*} b (bool):  true = OK; false = Error
 * @param {*} s (String): text to show
*****************************************************************************************/
export function setResponse(b, s) {
  try {
  	// clear if previous timer still run
    clearTimeout(timer);
  } catch(e) {}
  
  try {
    var r = document.getElementById("response");
    var t = 2000;
    if (!b) t = 5000; // show errors longer

    r.innerHTML = s;
    if (b) { r.className = "oktext"; } else {r.className = "errortext";}
    timer = setTimeout(function() {document.getElementById("response").innerHTML=""}, 2000);
  } catch(e) {}
}

/******************************************************************************************
 * definition of creating selectionlists from input fields
 * @param {*} querySelector -> select input fields to convert
 * @param {*} jsonLists -> define multiple predefined lists to set as option as array
 * @param {*}blacklist -> simple list of ports (numbers) to set as disabled option 
 * @example 
 * CreateSelectionListFromInputField('input[type=number][id^=AllePorts], input[type=number][id^=GpioPin]', [gpio, gpio_analog], gpio_disabled);
******************************************************************************************/
export function CreateSelectionListFromInputField(querySelector, jsonLists, blacklist) {
	var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll(querySelector);
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = document.createElement('select');
    _select.id = objects[j].id;
    _select.name = objects[j].name;
    for ( k = 0; k < jsonLists.length; k++ ) {  
      for ( i = 0; i < jsonLists[k].length; i++ ) {
          _option = document.createElement( 'option' );
          _option.value = jsonLists[k][i].port; 
          _option.text  = jsonLists[k][i].name;
          if(objects[j].value == jsonLists[k][i].port) { _option.selected = true;}
          if(blacklist && blacklist.indexOf(jsonLists[k][i].port)>=0) {
          	_option.disabled = true;
          }
          _select.add( _option ); 
      }
    }
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }
}

/*****************************************************************************************
 * returns, if a element is visible or not
****************************************************************************************/
function isVisible(_obj) {
	var ret = true;
	if (_obj && _obj.style.display == "none") { ret = false;}
  else if (_obj && _obj.parentNode && _obj.tagName != "HTML") ret = isVisible(_obj.parentNode);
  return ret;
}

/****************************************************************************************
separator: 
regex of item ID to identify first element in row
  - if set, returned json is an array, all elements per row, example: "^myonoffswitch.*"
  - if emty, all elements at one level together, ONLY for small json´s (->memory issue)
****************************************************************************************/
export function onSubmit(DataForm, separator='') {
  // init json Objects
  var JsonData, tempData; 
  
  if (separator.length == 0) { JsonData =  {data: {}}; }
  else { JsonData =  {data: []};}
  tempData = {};
  
  var elems = document.getElementById(DataForm).elements; 
  for(var i = 0; i < elems.length; i++){ 
    if(elems[i].name && elems[i].value) {
      if (!isVisible(elems[i])) { continue; }
      
      // tempData -> JsonData if new row (first named element (-> match) in row)
      if (separator.length > 0 && elems[i].id.match(separator) && Object.keys(tempData).length > 0) {
      	JsonData.data.push(tempData);
        tempData = {};
      } else if (separator.length == 0 && Object.keys(tempData).length > 0) {
        JsonData.data[Object.keys(tempData)[0]] = tempData[Object.keys(tempData)[0]];
        tempData = {};
      }
      
      if (elems[i].type == "checkbox") {
        tempData[elems[i].name] = (elems[i].checked==true?1:0);
      } else if (elems[i].id.match(/^Alle.*/) || 
                 elems[i].id.match(/^GpioPin.*/) || 
                 elems[i].id.match(/^AnalogPin.*/) || 
                 elems[i].type == "number") {
        tempData[elems[i].name] = parseInt(elems[i].value); 
      } else if (elems[i].type == "radio") {
        if (elems[i].checked==true) {tempData[elems[i].name] = elems[i].value;}
      } else {
        tempData[elems[i].name] = elems[i].value;
      }
    }
  } 
  
  // ende elements
  if (separator.length > 0 && Object.keys(tempData).length > 0) {
    JsonData.data.push(tempData);
    tempData = {};
  } else if (separator.length == 0 && Object.keys(tempData).length > 0) {
    JsonData.data[Object.keys(tempData)[0]] = tempData[Object.keys(tempData)[0]];
    tempData = {};
  }  

  setResponse(true, "save ...")

  var filename = document.location.pathname.replace(/^.*[\\/]/, '')
  filename = filename.substring(0, filename.lastIndexOf('.')) || filename // without extension
  
  var textToSaveAsBlob = new Blob([JSON.stringify(JsonData)], {type:"text/plain"});
  
  const formData = new FormData();
  formData.append(filename + ".json", textToSaveAsBlob, '/config/' + filename + ".json");
    
    fetch('/doUpload', {
      method: 'POST',
      body: formData,
    })
      .then (response => response.json())
      .then (json =>  {
        setResponse(true, json.text)
      })
      .then (() => {  
        var data = {};
        data['cmd'] = {};
        data['cmd']['action'] = "ReloadConfig";
        data['cmd']['subaction'] = filename;
        requestData(data);
      }); 
}


/****************************************************************************************
 * blendet Zeilen der Tabelle aus
 * @param {*} show: Array of shown IDs return true;
 * @param {*} hide: Array of hidden IDs 
 * @example radioselection(["row1", "row2"], ["row3", "row4"])
****************************************************************************************/
export function radioselection(show, hide) {
  for(var i = 0; i < show.length; i++){
    if (document.getElementById(show[i])) {document.getElementById(show[i]).style.display = 'table-row';}
  }
  for(var j = 0; j < hide.length; j++){
    if(document.getElementById(hide[j])) {document.getElementById(hide[j]).style.display = 'none';}
  }
}

/****************************************************************************************
 * Show elements on checkbox is set, otherwise hide elements
 * @param {*} checkbox object of the checkbox
 * @param {*} show Array of shown IDs if checkbox is checked
 * @param {*} hide Array of hidden IDs if checkbox is checked
 * @returns {*} void
 ****************************************************************************************/
export function onCheckboxSelection(checkbox, show, hide) {
  if (checkbox.checked) {
    radioselection(show, hide);
  } else {
    radioselection(hide, show);
  }
}

/****************************************************************************************
 * Transforms all checkboxes in the document that are not within a div with the style class "onoffswitch".
 * 
 * This function searches for all input elements of type checkbox that do not have the class "onoffswitch-checkbox".
 * For each of these checkboxes, it creates a new div element with the class "onoffswitch", clones the checkbox into this div,
 * and adds a label with the necessary span elements for styling. Finally, it replaces the original checkbox with the new div element.
 ****************************************************************************************/
export function transformCheckboxes() {
  // Alle Checkboxen im Dokument suchen deren elternelement kein div mit der Style class "onoffswitch" ist
  const checkboxes = document.querySelectorAll("input[type='checkbox']:not(.onoffswitch-checkbox)");

  for (var i = 0; i < checkboxes.length; i++) {
    // Eltern-Element der Checkbox
    const parent = checkboxes[i].parentElement;
    
    // Neues Div-Element erstellen
    const div = document.createElement('div');
    div.className = 'onoffswitch';

    // Checkbox in das neue Div-Element kopieren
    const newCheckbox = checkboxes[i].cloneNode(true)
    
    newCheckbox.className = 'onoffswitch-checkbox';
    div.appendChild(newCheckbox);
     
    // Neues Label-Element erstellen
    const label = document.createElement('label');
    label.className = 'onoffswitch-label';
    label.setAttribute('for', checkboxes[i].id);

    // Span-Elemente für das Label erstellen
    const spanInner = document.createElement('span');
    spanInner.className = 'onoffswitch-inner';

    const spanSwitch = document.createElement('span');
    spanSwitch.className = 'onoffswitch-switch';

    // Span-Elemente zum Label hinzufügen
    label.appendChild(spanInner);
    label.appendChild(spanSwitch);

    // Label zum Div-Element hinzufügen
    div.appendChild(label);
    
    // Neues Div-Element anstelle der ursprünglichen Checkbox einfügen
    parent.replaceChild(div, checkboxes[i]);

  }
}

/****************************************************************************************
 * Get all form data values as a string
 * @param {*} formElement: id of the form element
 * 
 * @returns {*} string containing all form data values
 * ****************************************************************************************/
export function getFormData(formElement) {
  const form = document.getElementById(formElement);
  if (form) {
    const formData = new FormData(form);
    let dataString = '';
    formData.forEach((value, key) => {
      dataString += `${key}=${value}|`;
    });
    // Remove the last '|' character
    dataString = dataString.slice(0, -1);
    return dataString;
  }
}

/****************************************************************************************
 * Show a dialog if the form data has been changed
 * ****************************************************************************************/
export function showMustSaveDialog() {
  if (document.getElementById('needToSave') && datavalues !== getFormData("DataForm")) {
    document.getElementById('needToSave').classList.remove('hide');
  } else {
    document.getElementById('needToSave').classList.add('hide');
  }
}

/****************************************************************************************
 * Initialize the data values in variable "datavalues"
 * ****************************************************************************************/
export function initDataValues() {
  datavalues = getFormData("DataForm");
  
  if (document.getElementById('needToSave')) {
    document.getElementById('needToSave').classList.add('hide');
  }
}
/****************************************************************************************
****************************************************************************************/
