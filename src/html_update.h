#ifndef HTMLUPDATE_H
#define HTMLUPDATE_H

const char HTML_UPDATERESPONSE[] PROGMEM = R"=====(

<!doctype html>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta charset="utf-8">
  <link rel="stylesheet" type="text/css" href="/web/Style.css">
  <script language="javascript" type="text/javascript">
    setTimeout(GoBack, 10000);
    function GoBack() {
        window.location.href="/";
    }
  </script>
  <title>Solar Inverter Modbus MQTT Gateway</title></head>
  <body>
    Success! Rebooting...
    <p/>
    <input class='button' type='button' value='Go Back' onclick="window.location.href='/';" />
  </body>
</html>

)=====";


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

      <form method='POST' action='/update' enctype='multipart/form-data'>
        <input type='file' accept='.bin,.bin.gz,.image' name='filesystem'>
        <input type='submit' value='Update Filesystem'>
      </form>
  -->

  <!--
      ESP crash
      https://github.com/me-no-dev/ESPAsyncWebServer/issues/1381

      <p>please select 'data' directory: 
        <input type="file" name='filesystem' onchange='UploadData(this)' webkitdirectory mozdirectory />
        </p><span id="response"></span>


        <script language='javascript' type='text/javascript'>
        function UploadData(item) {
            console.log("prepare "+item.files.length+" files for uploading");
        if (item.files.length > 0) {
            const formdata = new FormData();
            for (const file of item.files) {
                var filepath = file.webkitRelativePath;
            if (filepath.startsWith('data/')) {
                filepath = filepath.substr(4);
                    formdata.append(file.name, file, filepath);
                console.debug("File accepted: " + filepath);      
            } else {
                console.warn("File "+filepath+" not in data directory");
            }
            }
            var cnt = 0;
            var response = document.getElementById("response");
            
            for (const key of formdata.keys()) {
            console.log("file "+key+" uploaded");
            cnt++;
            }
            
            if (cnt > 0) {
            fetch('/doUpload', {
                method: 'POST',
                body: formdata,
            })
            .then (() => {   		
                response.innerHTML="upload successful of "+cnt+" files";
            })
            .catch(reason => {
                console.error(reason);
                response.innerHTML="upload failed: "+reason;
            });
            } else { response.innerHTML = "not the 'data' directory selected"; }
            }              
        }
        </script>
    -->

    </body></html>

)=====";

#endif