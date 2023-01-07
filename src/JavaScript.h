// jsfiddle.net

#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

const char JAVASCRIPT[] PROGMEM = R"=====(
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

//############ DO NOT CHANGE BELOW ###################

window.addEventListener('load', init, false);
function init() {
  SetConfiguredPorts();
  SetAvailablePorts();
}

function SetConfiguredPorts() {
  var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll('input[type=number][id^=ConfiguredPorts]');
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = document.createElement('select');
    _select.id = objects[j].id;
    _select.name = objects[j].name;
    for ( i = 0; i < configuredPorts.length; i += 1 ) {
        _option = document.createElement( 'option' );
        _option.value = configuredPorts[i].port; 
        _option.text  = configuredPorts[i].name;
        if(objects[j].value == configuredPorts[i].port) { _option.selected = true;}
        _select.add( _option ); 
    }
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }
}

function SetAvailablePorts() {
  var _parent, _select, _option, i, j, k;
  var objects = document.querySelectorAll('input[type=number][id^=AllePorts], input[type=number][id^=GpioPin]');
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = createGpioPortSelectionList(objects[j].id, objects[j].name, objects[j].value);
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }

  var objects = document.querySelectorAll('input[type=number][id^=AnalogPin]');
  for( j=0; j< objects.length; j++) {
    _parent = objects[j].parentNode;
    _select = createAnalogPortSelectionList(objects[j].id, objects[j].name, objects[j].value);
    _parent.removeChild( objects[j] );
    _parent.appendChild( _select );
  }
}

function createGpioPortSelectionList(id, name, value) {
  _select = document.createElement('select');
  _select.id = id;
  _select.name = name;
  for ( i = 0; i < gpio.length; i += 1 ) {
    // alle GPIO Pins in die Liste
    _option = document.createElement( 'option' );
    _option.value = gpio[i].port; 
    if(gpio_disabled.indexOf(gpio[i].port)>=0) {_option.disabled = true;}
    if(value == (gpio[i].port)) { _option.selected = true;}
    _option.text  = gpio[i].name;
    _select.add( _option ); 
  }
  if (id.match(/^Alle.*/)) {
    // Alle PCF Ports in die Liste wenn ID match "Alle*"
    for ( k = 0; k < availablePorts.length; k++ ) {
      _option = document.createElement( 'option' );
      _option.value = _option.text = availablePorts[k];
      if(value == availablePorts[k]) { _option.selected = true;}
      _select.add( _option );
    }
  }
  return _select;
}
function createAnalogPortSelectionList(id, name, value) {
  _select = document.createElement('select');
  _select.id = id;
  _select.name = name;
  for ( i = 0; i < gpioanalog.length; i += 1 ) {
    // alle GPIO Pins in die Liste
    _option = document.createElement( 'option' );
    _option.value = gpioanalog[i].port; 
    if(value == (gpioanalog[i].port)) { _option.selected = true;}
    _option.text  = gpioanalog[i].name;
    _select.add( _option ); 
  }
  
  return _select;
}

function ShowError(t){
  if(t && t.length>0) { t += '<br>Breche Speichervorgang ab. Es wurde nichts gespeichert!' }
  document.getElementById('ErrorText').innerHTML = t;
}

function onSubmit(DataForm, SubmitForm){
  // erstelle json String
  var formData = {};
  ShowError('');
  
  var elems = document.getElementById(DataForm).elements; 
  for(var i = 0; i < elems.length; i++){ 
    if(elems[i].name && elems[i].value) {
      if (elems[i].style.display == 'none') {continue;}
      if (elems[i].parentNode.tagName == 'DIV' && elems[i].parentNode.style.display == 'none') {continue;}
      if (elems[i].parentNode.parentNode.tagName == 'TR' && elems[i].parentNode.parentNode.style.display == 'none') {continue;}
      
      if (elems[i].type == "checkbox") {
        formData[elems[i].name] = (elems[i].checked==true?1:0);
      } else if (elems[i].id.match(/^Alle.*/) || elems[i].id.match(/^GpioPin.*/) || elems[i].id.match(/^AnalogPin.*/) || elems[i].type == "number") {
        formData[elems[i].name] = parseInt(elems[i].value);
      } else if (elems[i].type == "radio") {
        if (elems[i].checked==true) {formData[elems[i].name] = elems[i].value;}
      } else {
        formData[elems[i].name] = elems[i].value;
      }
    }
  } 
  formData["count"] = document.getElementById(DataForm).getElementsByClassName('editorDemoTable')[0].rows.length -1;
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

/* https://jsfiddle.net/tobiasfaust/p5q9hgsL/ */
/*******************************
reset of rawdata views
*******************************/
function reset_rawdata(rawdatatype) {
  var string_rawdata = document.getElementById(rawdatatype + '_org').innerHTML;
  var bytes = string_rawdata.split(" ");

	bytes = prettyprint_rawdata(bytes);
  document.getElementById(rawdatatype).innerHTML = bytes.join(' ');
}

/*******************************
insert Tooltips and linebreaks
*******************************/
function prettyprint_rawdata(bytearray) {
	for( i=0; i< bytearray.length; i++) {
  	bytearray[i] = '<dfn class=\'tooltip\'>' + bytearray[i] + '<span role=\'tooltip\'>Position: ' + i + '</span></dfn>';
    if (i % 10 == 0) {
      bytearray[i] = ' <br>' + bytearray[i];
    }
  }
  return bytearray;
}

/*******************************
compute result from selected positions
*******************************/
function check_rawdata() {
  var datatype = document.querySelector('input[name="datatype"]:checked').value;
  var rawdatatype = document.querySelector('input[name="rawdatatype"]:checked').value;
  var string_positions = document.getElementById('positions').value;
  var string_rawdata = document.getElementById(rawdatatype + '_org').innerHTML;
  
  const bytes = string_rawdata.split(" ");
  const pos = string_positions.split(",");
  
  // reset all rawdata containers
	reset_rawdata('id_rawdata');
  reset_rawdata('live_rawdata');
  
	var result;
  if (datatype == 'int') { result = 0; }
  if (datatype == 'string') { result = "";}
  
	for( j=0; j< pos.length; j++) {
    if (datatype == 'int') { 
    	result = result << 8 | parseInt(Number(bytes[Number(pos[j])]), 10);
    }
		if (datatype == 'string') { 
    	result = result + String.fromCharCode(parseInt(bytes[Number(pos[j])]), 16); 
    }
    
   bytes[Number(pos[j])] = "<span style='color: red;'>" + bytes[Number(pos[j])] + "</span>";
  }
  
  document.getElementById('rawdata_result').innerHTML = "= " + result;
  document.getElementById(rawdatatype).innerHTML = prettyprint_rawdata(bytes).join(' ');
}

)=====";

#endif
