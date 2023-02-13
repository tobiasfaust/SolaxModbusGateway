#ifndef HTMLUPDATE_H
#define HTMLUPDATE_H

const char HTML_UPDATERESPONSE[] PROGMEM = R"=====(

<html><head><meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta charset="utf-8">
<link rel="stylesheet" type="text/css" href="/style.css">
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
</body></html>

)=====";

const char HTML_UPDATEPAGE[] PROGMEM = R"=====(

<html><head><meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta charset="utf-8">
<link rel="stylesheet" type="text/css" href="/style.css">
<title>Solar Inverter Modbus MQTT Gateway</title></head>
<body>
  Update firmware:<p/>
  <form method='POST' action='/update' enctype='multipart/form-data'>
    <input type='file' name='update'>
    <input type='submit' value='Update'>
  </form>
</body></html>

)=====";

#endif