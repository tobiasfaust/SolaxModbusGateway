/*############################################################
#
# definition of constants
#
############################################################*/

const gpio_disabled = [];

const gpio = [  {port: 2 , name:'D2'},
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

const gpioanalog = [  {port: 36, name:'ADC1_CH0 - GPIO36'},
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

/*############################################################
#
# central function to initiate data fetch
#
############################################################*/

function requestData(json, highlight) {
  const data = new URLSearchParams();
  data.append('json', json);

  fetch('/ajax', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: data
  })
  .then (response => response.json())
  .then (json =>  { handleJsonItems(json, highlight)}); 
}

/*############################################################
#
# definition of applying jsondata to html templates
#
############################################################*/

function applyKeys(json, _tpl, counter, highlight) {
		for (var key in json) {
      if (typeof json[key] === 'object') {
      	applyTemplate((json[key]), key, _tpl, highlight);
      } else {
      	try {
          var _obj;
          if (document.getElementById(_tpl.getElementById(key).id + "_" + counter)) {
          	// exists already in DOM
            _obj = document.getElementById(_tpl.getElementById(key).id + "_" + counter);
          } else {
	          _obj = _tpl.getElementById(key);
          }
          if (counter >= 0) { _obj.id = _obj.id + "_" + counter;}
          if (['SPAN', 'DIV', 'TD'].includes(_obj.tagName)) {
            if (highlight) {
              _obj.innerHTML = "<span class='ajaxchange'>" + json[key] + "</span>";
            } else {
              _obj.innerHTML = json[key];
            }
          } else {
            _obj.value = json[key];
          }
        } catch(e) {}
      }
    }
}

function applyTemplate(TemplateJson, templateID, doc, highlight) {
	if (Array.isArray(TemplateJson)) {
    for (var i=0; i < TemplateJson.length; i++) {
      var _tpl = document.importNode(doc.getElementById(templateID).content, true);
      var _parentObj = doc.getElementById(templateID).parentNode;
      
      try {
      	_tpl.getElementById('id').id = templateID + "_" + [i];
      } catch(e) {}
      
      applyKeys(TemplateJson[i], _tpl, i, highlight);
      
      if (document.getElementById(templateID + "_" + [i])) {
      	// item already in DOM
      } else { 
	      _parentObj.appendChild(_tpl);
      }
      
    }
  } else {
  	var _tpl = document.importNode(doc.getElementById(templateID).content, true);
    var _parentObj = doc.getElementById(templateID).parentNode;

    applyKeys(TemplateJson[i], _tpl, undefined, highlight);  

    _parentObj.appendChild(_tpl);
  }
}

function handleJsonItems(json, highlight) {
  if ("data" in json) {
  	applyKeys(json.data, document, undefined, highlight);
  }
  
  if ('response' in json) {
    try {
      if (json.response.status == 1) {setResponse(true, json.response.text);}
      if (json.response.status == 0) {setResponse(false, json.response.text);}
    } catch(e) {setResponse(false, 'unknow error');}
  }
}

// ***********************************
// show response
// b => bool => true = OK; false = Error
// s => String => text to show
// ***********************************
function setResponse(b, s) {
  try {
    var r = document.getElementById("response");
    r.innerHTML = s;
    if (b) { r.className = "oktext"; } else {r.className = "errortext";}
    setTimeout(function() {document.getElementById("response").innerHTML=""}, 2000);
  } catch(e) {}
}

/*############################################################
#
# definition of creating selectionlists from input fields
# querySelector -> select input fields to convert
# jsonLists -> define multiple predefined lists to set as option as array
# blacklist -> simple list of ports (numbers) to set as disabled option 
#
# example: 
# CreateSelectionListFromInputField('input[type=number][id^=AllePorts], input[type=number][id^=GpioPin]', 
#                                    [gpio, gpio_analog], gpio_disabled);
############################################################*/
function CreateSelectionListFromInputField(querySelector, jsonLists, blacklist) {
	var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll(querySelector);
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = document.createElement('select');
    _select.id = objects[j].id;
    _select.name = objects[j].name;
    for ( k = 0; k < jsonLists.length; k += 1 ) {  
      for ( i = 0; i < jsonLists[k].length; i += 1 ) {
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

/*############################################################
# DataForm: id of FormElement which contains all input fields
# SubmitForm: id of FormElement which contains hidden input filed, named "json", for submit
# jsontype:
#   1 = standard, 1 level
#   2 = array, each item separately
############################################################*/
function onSubmit(DataForm, SubmitForm, jsontype){
  //set default
  if (!jsontype) jsontype = 1;
  
  // init json String
  var formData; 
  if (jsontype == 1) { formData =  {}; }
  else if (jsontype == 2) { formData =  {data: []}; }
  
  var count = 0;
  
  var elems = document.getElementById(DataForm).elements; 
  for(var i = 0; i < elems.length; i++){ 
    if(elems[i].name && elems[i].value) {
      if (elems[i].style.display == 'none') {continue;}
      if (elems[i].parentNode.tagName == 'DIV' && elems[i].parentNode.style.display == 'none') {continue;}
      if (elems[i].parentNode.parentNode.tagName == 'TR' && elems[i].parentNode.parentNode.style.display == 'none') {continue;}
      
      if (elems[i].type == "checkbox") {
        count++;
        if (jsontype == 1) { formData[elems[i].name] = (elems[i].checked==true?1:0); }
        else if (jsontype == 2) {
          formData.data.push({ "name" : elems[i].name,
                               "value": (elems[i].checked==true?1:0) });
        }
      } else if (elems[i].id.match(/^Alle.*/) || 
                 elems[i].id.match(/^GpioPin.*/) || 
                 elems[i].id.match(/^AnalogPin.*/) || 
                 elems[i].type == "number") {
        count++;
        if (jsontype == 1) { formData[elems[i].name] = parseInt(elems[i].value); }
        else if (jsontype == 2) {
          formData.data.push({ "name" : elems[i].name,
                                "value": parseInt(elems[i].value) });
        }
      } else if (elems[i].type == "radio") {
        if (jsontype == 1) { if (elems[i].checked==true) {formData[elems[i].name] = elems[i].value;} }
        else if (jsontype == 2) {
          if (elems[i].checked==true) {
            count++;
            formData.data.push({ "name" : elems[i].name,
                                 "value": elems[i].value });
           }
        }
      } else {
        if (jsontype == 1) { formData[elems[i].name] = elems[i].value; }
        else if (jsontype == 2) {
          count++;
          formData.data.push({ "name" : elems[i].name,
                               "value": elems[i].value });
        }
      }
    }
  } 
  formData.count = count;
  json = document.getElementById(SubmitForm).querySelectorAll("input[name='json']");

  if (json[0].value.length <= 3) {
    json[0].value = JSON.stringify(formData);
  }
 
  return true;
}

/*******************************
blendet Zeilen der Tabelle aus
  a: Array of shown IDs 
  b: Array of hidden IDs 
*******************************/
function radioselection(a,b) {
  for(var i = 0; i < a.length; i++){
    if (document.getElementById(a[i])) {document.getElementById(a[i]).style.display = 'table-row';}
  }
  for(var j = 0; j < b.length; j++){
    if(document.getElementById(b[j])) {document.getElementById(b[j]).style.display = 'none';}
  }
}

