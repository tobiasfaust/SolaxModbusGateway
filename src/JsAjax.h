#ifndef AJAX_H
#define AJAX_H

const char JSAJAX[] PROGMEM = R"=====(

var myInterval = setInterval(RefreshLiveData, 5000);

function RefreshLiveData() {
  var data = {};
  data['action'] = "RefreshLiveData";
  ajax_send(JSON.stringify(data));
}

function ajax_send(json) {
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
  
  //http.onerror = function(){ alert (http.responseText); }
  //http.onload = function(){ alert (http.responseText); }
  
  http.onreadystatechange = function() { //Call a function when the state changes.
      if(http.readyState == 4 && http.status == 200) {
          var res = JSON.parse(http.responseText);
          
          Object.entries(res).forEach((entry) => {
            const [key, value] = entry;
            
            obj = document.getElementById(key);
            obj.innerHTML = "<span class='ajaxchange'>"+value+"</span>";

          });
      } 
    }
  http.send(params);
}


)=====";

#endif
