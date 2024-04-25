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
  requestData(JSON.stringify(data));
}

// ************************************************
function highlightNavi(item) {
  collection = document.getElementsByName('navi')
  
  for (let i = 0; i < collection.length; i++) {
    if (item.id == collection[i].id ) {
      document.getElementById(collection[i].id).classList.add('navi_active');
    } else {
      document.getElementById(collection[i].id).classList.remove('navi_active');
    }

  }
}

// ************************************************