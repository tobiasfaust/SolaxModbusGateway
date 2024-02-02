var myInterval = setInterval(RefreshLiveData, 5000);

function ChangeActiveStatus(id) {
  obj = document.getElementById(id);
  item = id.replace(/^activeswitch_(.*)$/g, "$1");
  var data = {};
  data['action'] = "SetActiveStatus";
  data['newState'] = (obj.checked?"true":"false");
  data['item'] = item;
  
  ajax_send(JSON.stringify(data));
}

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
          for ( var i = 0; i < res["data"].length; i++ ) {
            //alert(res["data"][i]["name"])
            try {
              obj = document.getElementById(res["data"][i]["name"]);
              obj.innerHTML = "<span class='ajaxchange'>" + res["data"][i]["value"] + "</span>";
            } catch(e) {}
          }
      } 
    }
  http.send(params);
}
