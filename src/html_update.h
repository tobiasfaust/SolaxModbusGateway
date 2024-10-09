#ifndef HTMLUPDATE_H
#define HTMLUPDATE_H

// https://jsfiddle.net/tobiasfaust/Lc1earnz/
const char HTML_UPDATEPAGE[] PROGMEM = R"=====(

<!doctype html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta charset="utf-8">
    <link rel="stylesheet" type="text/css" href="/web/Style.css">
    <title>Solar Inverter Modbus MQTT Gateway</title></head>
    <body>
      Update firmware:<p/>
      <form method='POST' action='/update' enctype='multipart/form-data'>
        <input type='file' accept='.bin,.bin.gz' name='firmware'>
        <input type='submit' value='Update Firmware'>
      </form>
  <!--    
      ESP32 Lib kennt kein U_LITTLEFS
      https://github.com/espressif/arduino-esp32/issues/6482
      FeatureRequest ESP32:
      https://github.com/espressif/arduino-esp32/issues/9347

      <form method='POST' action='/update' enctype='multipart/form-data'>
        <input type='file' accept='.bin,.bin.gz,.image' name='filesystem'>
        <input type='submit' value='Update Filesystem'>
      </form>
  -->

  <!--
      ESP crash
      https://github.com/me-no-dev/ESPAsyncWebServer/issues/1381
-->
      <p>please select 'data' directory: 
        <input type="file" name='filesystem' onchange='UploadData(this)' webkitdirectory mozdirectory />
        </p><span id="response"></span>


        <script language='javascript' type='text/javascript'>
        function UploadData (item) {
        	var files = [];
          console.log("prepare "+item.files.length+" files for uploading");
          if (item.files.length > 0) {
          	UploadFile(item.files, 0);
          }        
        }
        
        function UploadFile(files, number) {
        	var response = document.getElementById("response");
          
          if (number < files.length) {
          	const formdata = new FormData();
            var filepath = files[number].webkitRelativePath;
            
            if (filepath.startsWith('data/')) {
              filepath = filepath.substr(4);
              formdata.append(files[number].name, files[number], filepath);
              console.debug("File accepted: " + filepath); 
              
              fetch('/doUpload', {
                    method: 'POST',
                    body: formdata,
              })
              .then (() => {   		
                    response.innerHTML += "upload successful of "+files[number].name+"<br>";
                    setTimeout(UploadFile, 1000, files, number +1);
              })
              .catch(reason => {
                    console.error(reason);
                    response.innerHTML += "upload failed of " + files[number].name + ": " + reason +"<br>";
                    UploadFile(files, number +1);
              });
                
            } else {
              console.warn("File " + filepath + " not in data directory");
              UploadFile(files, number +1);
            }
            
          } else {
          	response.innerHTML += "<br>Upload finished.....<br>"
          }
        }
        </script>
    </body>
</html>

)=====";

#endif