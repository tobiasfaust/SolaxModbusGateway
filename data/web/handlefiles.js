// https://jsfiddle.net/tobiasfaust/uc1jfpgb/

import * as global from './Javascript.js';

var DirJson;

// ************************************************
export const functionMap = {
  files_Callback: MyCallback
};

// ************************************************
export function init1() {
  var data = {"JS": {"listdir": [
      {"path": "/", "content": [{"name": "file1.txt", "isDir": 0}, {"name": "file2.txt", "isDir": 0}, {"name": "config", "isDir": 1}]},
      {"path": "/config", "content": [{"name": "file3.txt", "isDir": 0}, {"name": "file4.txt", "isDir": 0}]}
    ]},
    "response": {"status": 1, "text": "successful"},
    "cmd": {"callbackFn": "files_Callback", "startpath": "/"}
    };

  global.handleJsonItems(data);

  document.getElementById('fullpath').innerHTML = ''; // div 
  document.getElementById('filename').value = ''; // input field
  document.getElementById('content').value = '';

  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

export function init() {
  // Initiale Verbindung aufbauen
  global.connectWebSocket();

  // Warte bis die WebSocket-Verbindung aufgebaut ist
  let checkWebSocketInterval = setInterval(() => {
    if (global.ws && global.ws.readyState === WebSocket.OPEN) {
      clearInterval(checkWebSocketInterval);
      GetInitData("/");
    }
  }, 100);
}

// ************************************************
export function GetInitData(startpath) {
  var data = {};
  data['cmd'] = {};
  data['cmd']['action'] = "handlefiles";
  data['cmd']['subaction'] = "listDir"
  data['cmd']['startpath'] = startpath;
  data['cmd']['callbackFn'] = "files_Callback";

  global.requestData(data);

  document.getElementById('fullpath').innerHTML = ''; // div 
  document.getElementById('filename').value = ''; // input field
  document.getElementById('content').value = '';

  document.querySelector("#loader").style.visibility = "hidden";
  document.querySelector("body").style.visibility = "visible";
}

// ************************************************
function MyCallback(json) {
  DirJson = json["JS"].listdir;
  listFiles(json["cmd"]["startpath"]);
}

// ************************************************
// show content of fetched file 
// ************************************************
function setContent(string, file) {
  document.getElementById('fullpath').innerHTML = file; // div 
  document.getElementById('filename').value = basename(file); // input field
  
  if (file.endsWith("json")) {
    document.getElementById('content').value = JSON.stringify(JSON.parse(string), null, 2);
  } else {
    document.getElementById('content').value = string;
  }
}

// ***********************************
// fetch file from host
// ***********************************
export function fetchFile(file) {
  document.getElementById('content').value = "loading "+file+"...";
  
  fetch(file)
  .then(response => response.text())
  .then(textString => setContent(textString, file));
}

// ***********************************
// show directory structure 
// ***********************************
export function listFiles(path) {
  var table = document.querySelector('#files'),
      row = document.querySelector('#NewRow'),
      cells, tr_tpl, DirJsonLocal;
  
  // cleanup table
  table.replaceChildren();
  
  // get the right part
  for(let i = 0; i < DirJson.length; i++) { 
    if (DirJson[i].path == path) {
      DirJsonLocal = DirJson[i]
    }
  }

  // show path information
  document.getElementById('path').innerHTML = path;
  
  // set "back" item if not root
  if (path != '/') {
    tr_tpl = document.importNode(row.content, true);
    cells = tr_tpl.querySelectorAll("td");
      cells.forEach(function (item, index) {
        var text = item.innerHTML;
        var oc = "listFiles('" + getParentPath(path) + "')"
        text = text.replaceAll("{file}", '..');
        item.innerHTML = text;
        item.setAttribute('onClick', oc);
    });
    table.appendChild(tr_tpl);
  }
  
  // show files
  DirJsonLocal.content.forEach(function (file) {
    // template "laden" (lies: klonen)
    tr_tpl = document.importNode(row.content, true);
    cells = tr_tpl.querySelectorAll("td");
    cells.forEach(function (item, index) {
      var text = item.innerHTML;
      var oc;
      
      if(file.isDir == 0) {
        oc = item.getAttribute('onClick');
        var newPath = DirJsonLocal.path + "/" + file.name;
        if (newPath.startsWith("//")) {newPath = newPath.substring(1)}
        oc = oc.replaceAll("{fullpath}", newPath);
        text = text.replaceAll("{file}", file.name);
      } else if(file.isDir == 1) {
        var newPath = DirJsonLocal.path + "/" + file.name;
        if (newPath.startsWith("//")) {newPath = newPath.substring(1)}
        oc = "listFiles('" + newPath + "')"
        text = text.replaceAll("{file}", file.name + "/");
      }
      
      
      item.innerHTML = text;
      item.setAttribute('onClick', oc);
    });
    table.appendChild(tr_tpl);
  })
 }

// ***********************************
// returns parent path: '/regs/web' -> '/regs' 
// ***********************************
function getParentPath(path) {
  var ParentPath, PathArray;
  
  PathArray = path.split('/')
  PathArray.pop()
  if (PathArray.length == 1) { ParentPath = '/' }
  else { ParentPath = PathArray.join('/')}
  return ParentPath
}

// ***********************************
// extract the filename from path
// ***********************************
function basename(str) {
    return str.split('\\').pop().split('/').pop();
}

// ***********************************
// return true if valid json, otherwise false
// ***********************************
function validateJson(json) {
  try {
    JSON.parse(json);
    return true;
  } catch {
      return false;
  }
}

// ***********************************
// download content of textarea as filename on local pc
// ***********************************
export function downloadFile() {
  var textToSave = document.getElementById("content").value;
  var textToSaveAsBlob = new Blob([textToSave], {type:"text/plain"});
  var textToSaveAsURL = window.URL.createObjectURL(textToSaveAsBlob);
  var fileNameToSaveAs = document.getElementById("filename").value;
 
  if (fileNameToSaveAs != '') { 
    var downloadLink = document.createElement("a");
    downloadLink.download = fileNameToSaveAs;
    downloadLink.innerHTML = "Download File";
    downloadLink.href = textToSaveAsURL;

    downloadLink.onclick = destroyClickedElement;
    downloadLink.style.display = "none";
    document.body.appendChild(downloadLink);
    downloadLink.click();
  } else { global.setResponse(false, 'Filename is empty, Please define it.');}
}

function destroyClickedElement(event)
{
    document.body.removeChild(event.target);
}

// ***********************************
// store content of textarea
// ***********************************
export function uploadFile() {
  var textToSave = document.getElementById("content").value;
  var textToSaveAsBlob = new Blob([textToSave], {type:"text/plain"});
  var fileNameToSaveAs = document.getElementById("filename").value;
  var pathOfFile = document.getElementById('path').innerHTML;
  
  if (fileNameToSaveAs != '') {
    if (fileNameToSaveAs.toLowerCase().endsWith('.json')) {
      if (!validateJson(textToSave)) {
        global.setResponse(false, 'Json invalid')
        return;
      }
    }

    global.setResponse(true, 'Please wait for saving ...');
    
    const formData = new FormData();
    formData.append(fileNameToSaveAs, textToSaveAsBlob, pathOfFile + '/' + fileNameToSaveAs);
    
    fetch('/doUpload', {
      method: 'POST',
      body: formData,
    })
      .then (response => response.json())
      .then (json =>  {
        global.setResponse(true, json.text)
      }); 
  
  } else { global.setResponse(false, 'Filename is empty, Please define it.');}
}

// ************************************************
export function deleteFile() {
  var pathOfFile = document.getElementById('path').innerHTML;
  var fileName = document.getElementById("filename").value;
  
  if (fileName != '') {
    var data = {};
    data['cmd'] = {};
    data['cmd']['action'] = 'handlefiles';
    data['cmd']['subaction'] = "deleteFile";
    data['cmd']['filename'] = pathOfFile + '/' + fileName;

    global.setResponse(true, 'Please wait for deleting ...');
    global.requestData(data);
    GetInitData(pathOfFile);
  } else { global.setResponse(false, 'Filename is empty, Please define it.');}
}

// ************************************************
export function deleteFolder() {
  var pathOfFile = document.getElementById('path').innerHTML;
  
  if (pathOfFile != '/') {
    var data = {};
    data['cmd'] = {};
    data['cmd']['action'] = 'handlefiles';
    data['cmd']['subaction'] = "deleteFolder";
    data['cmd']['foldername'] = pathOfFile;

    global.setResponse(true, 'Please wait for deleting ...');
    global.requestData(data);
    GetInitData(getParentPath(pathOfFile));
  } else { global.setResponse(false, 'You cannot delete the root folder.');}
}

// ************************************************
export function addFolder(folderName) {
  var pathOfFile = document.getElementById('path').innerHTML;

  if (folderName != '') {
    var data = {};
    data['cmd'] = {};
    data['cmd']['action'] = 'handlefiles';
    data['cmd']['subaction'] = "addFolder";
    data['cmd']['foldername'] = pathOfFile + '/' + folderName;

    global.setResponse(true, 'Please wait for adding ...');
    global.requestData(data);
    GetInitData(pathOfFile);
  } else { global.setResponse(false, 'Folder name is empty, Please define it.');}
}

// ***********************************
// backup complete filesystem of ESP by zipfile
//
// https://gist.github.com/noelvo/4502eea719f83270c8e9
// ***********************************
export function backup() {
  var url = [];
  
  for(let i = 0; i < DirJson.length; i++) { 
    DirJson[i].content.forEach(function (file) {
      //console.log("file: ", DirJson[i].path, file.name);
      if (file.isDir == 0 && DirJson[i].path.toLowerCase().startsWith("/config")) {
        url.push(DirJson[i].path + "/" + file.name);
      }
    })
  }
  handleZipDownload(url, "backup");
}

async function handleZipDownload(filePaths, zipFileName) {
  const zip = new JSZip();

  for (const filePath of filePaths) {
    try {
      const response = await fetch(filePath);
      if (!response.ok) {
        global.setResponse(false, 'Failed to fetch file: ' + filePath);
        console.error(`Failed to fetch ${filePath}: ${response.statusText}`);
        continue;
      }
      const fileContent = await response.blob();
      const relativePath = filePath.startsWith("/") ? filePath.substring(1) : filePath;
      zip.file(relativePath, fileContent);
    } catch (error) {
      global.setResponse(false, 'Error fetching file: ' + filePath);
      console.error(`Error fetching ${filePath}:`, error);
    }
  }

  zip.generateAsync({ type: "blob" }).then((content) => {
    const downloadLink = document.createElement("a");
    downloadLink.href = URL.createObjectURL(content);
    downloadLink.download = `${zipFileName}.zip`;
    downloadLink.style.display = "none";
    document.body.appendChild(downloadLink);
    downloadLink.click();
    document.body.removeChild(downloadLink);
  });
}

export async function handleZipUpload(event) {
  const file = event.target.files[0];
  if (!file) return;

  const zip = await JSZip.loadAsync(file);
  for (const [relativePath, zipEntry] of Object.entries(zip.files)) {
    if (!zipEntry.dir) {
      const fileContent = await zipEntry.async("blob");
      console.log("Uploading:", relativePath);
      const formData = new FormData();
      formData.append("file", fileContent, relativePath);

      try {
        const response = await fetch('/doUpload', {
          method: 'POST',
          body: formData
        });

        if (!response.ok) {
          global.setResponse(false, `Failed to upload ${relativePath}: ${response.statusText}`);
          console.error(`Failed to upload ${relativePath}: ${response.statusText}`);
        } else {
          global.setResponse(true, `Uploaded ${relativePath} successfully.`);
          console.log(`Uploaded ${relativePath} successfully.`);
        }
      } catch (error) {
        global.setResponse(false, `Error uploading ${relativePath}: ${error.message}`);
        console.error(`Error uploading ${relativePath}:`, error);
      }
    }
  }
}