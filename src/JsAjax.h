#ifndef AJAX_H
#define AJAX_H

const char JSAJAX[] PROGMEM = R"=====(

function ChangeEnabled(id) {
  obj = document.getElementById(id);
  num = id.replace(/^.*_(\d+)$/g, "$1");
  var data = {};

  data['action'] = "EnableValve"
  data['newState'] = obj.checked;

  port = document.getElementById('AllePorts_'+num); //'AllePorts_'+num
  if (port && port.style.display != 'none') { data['port'] = port.value; }
  port = document.getElementById('AllePorts_PortA_'+num);
  if (port && port.style.display != 'none') { data['port'] = port.value; }
  
  ajax_send(null, JSON.stringify(data));
}

function ChangeValve(id) {
  btn = document.getElementById(id);
  num = id.replace(/^action_(\d+).*/g, "$1");
  var data = {};
  
  data['action'] = "SetValve";
  data['newState'] = btn.value.replace(/^Set\ (.*)/, "$1");
  
  port = document.getElementById('AllePorts_'+num); //'AllePorts_'+num
  if (port && port.style.display != 'none') { data['port'] = port.value; }
  port = document.getElementById('AllePorts_PortA_'+num);
  if (port && port.style.display != 'none') { data['port'] = port.value; }
  
  ajax_send(btn, JSON.stringify(data));
}

function InstallRelease() {
  rel = document.getElementById('releases').value;
  var data = {}; 
  data['action'] = "InstallRelease";
  data['newState'] = rel;
  ajax_send(null, JSON.stringify(data));
}

function RefreshReleases() {
  var data = {}; 
  data['action'] = "RefreshReleases";
  data['newState'] = "";
  ajax_send(null, JSON.stringify(data));
}

function RefreshI2C(id) {
  btn = document.getElementById(id);
  
  var data = {};
  data['action'] = "RefreshI2C";
  data['newState'] = "";
  ajax_send(btn, JSON.stringify(data));
}

function Refresh1Wire(id) {
  btn = document.getElementById(id);
  
  var data = {};
  data['action'] = "Refresh1Wire";
  data['newState'] = "";
  ajax_send(btn, JSON.stringify(data));
}

function ajax_send(btn, json) {
  var http = null;
  ShowError("");
  if (window.XMLHttpRequest)  { http =new XMLHttpRequest(); }
  else                        { http =new ActiveXObject("Microsoft.XMLHTTP"); }
  
  if(!http){ alert("AJAX is not supported."); return; }
 
  var url = 'ajax';
  var params = 'json='+json;
  
  http.open('POST', url, true);
  
  //Send the proper header information along with the request
  http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  http.setRequestHeader('Content-length', params.length);
  http.setRequestHeader('Connection', 'close');
  
  //http.onerror = function(){ alert (http.responseText); }
  //http.onload = function(){ alert (http.responseText); }
  
  http.onreadystatechange = function() {//Call a function when the state changes.
      if(http.readyState == 4 && http.status == 200) {
          var jsonReturn = JSON.parse(http.responseText);
          if (btn.type == "button") {
            switch (jsonReturn.NewState) {
              case 'On':
                btn.value = 'Set Off';
                break;
              case 'Off':
                btn.value = 'Set On';
             }
           }
           else if (btn.tagName == "DIV") { 
            btn.innerHTML = "<span class='ajaxchange'>"+jsonReturn.NewState+"</span>";
           }
           
           if (jsonReturn.accepted == 0 && jsonReturn.error) { ShowError(jsonReturn.error); }
      } 
    }
  http.send(params);
}


)=====";

#endif
