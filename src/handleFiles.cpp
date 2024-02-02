#include "handleFiles.h"

handleFiles::handleFiles(AsyncWebServer *server) {

  server->on("/doUpload", HTTP_POST, [](AsyncWebServerRequest *request) {},
                                    std::bind(&handleFiles::handleUpload, this, std::placeholders::_1, 
                                        std::placeholders::_2,
                                        std::placeholders::_3,
                                        std::placeholders::_4,
                                        std::placeholders::_5,
                                        std::placeholders::_6));

}  

//###############################################################
// returns the complete folder structure
//###############################################################
void handleFiles::getDirList(JsonArray* json, String path) {
  StaticJsonDocument<128> doc;
  JsonObject jsonRoot = doc.to<JsonObject>();

  jsonRoot["path"] = path;
  JsonArray content = jsonRoot.createNestedArray("content");

  File FSroot = LittleFS.open(path);
  File file = FSroot.openNextFile();

  while (file) {
    StaticJsonDocument<64> doc1;
    JsonObject jsonObj = doc1.to<JsonObject>();
    String fname(file.name());
    jsonObj["name"] = fname;

    if(file.isDirectory()){    
        jsonObj["isDir"] = 1;
        String p = path + "/" + fname;
        if (p.startsWith("//")) { p = p.substring(1); }
        this->getDirList(json, p); // recursive call
    } else {
      jsonObj["isDir"] = 0;
    }

    content.add(jsonObj);
    file.close();
    file = FSroot.openNextFile();
  }
  FSroot.close();
  json->add(jsonRoot);
}

//###############################################################
// returns the requested data via AJAX from Webserver.cpp
//###############################################################
void handleFiles::HandleAjaxRequest(JsonDocument& jsonGet, AsyncResponseStream* response) {
  String subAction = "";
  if (jsonGet.containsKey("subAction"))  {subAction  = jsonGet["subAction"].as<String>();}

  if (Config->GetDebugLevel() >= 3) {
    Serial.printf("handle Ajax Request in handleFiles.cpp: %s\n", subAction);
  }

  if (subAction == "listDir") {
    StaticJsonDocument<256> doc;
    JsonArray content = doc.createNestedArray();
    
    this->getDirList(&content, "/");
    String ret("");
    serializeJson(content, ret);
    if (Config->GetDebugLevel() >= 5) {
      serializeJsonPretty(content, Serial);
      Serial.println();
    }
    response->print(ret);   
  }
}

//###############################################################
// store a file at Filesystem
//###############################################################
void handleFiles::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  AsyncResponseStream *response = request->beginResponseStream("text/json");
  response->addHeader("Server","ESP Async Web Server");
  StaticJsonDocument<256> jsonReturn;
  String ret;

  if (Config->GetDebugLevel() >=5) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
  }

  if (!index) {
    // open the file on first call and store the file handle in the request object
    request->_tempFile = LittleFS.open("/" + filename, "w");
    if (Config->GetDebugLevel() >=5) {
      String logmessage = "Upload Start: " + String(filename);
      Serial.println(logmessage);
    }
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    if (Config->GetDebugLevel() >=5) {
      String logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }
  }

  if (final) {
    // close the file handle as the upload is now done
    request->_tempFile.close();
    if (Config->GetDebugLevel() >=5) {
      String logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      Serial.println(logmessage);
    }

    jsonReturn["status"] = 1;
    jsonReturn["text"] = "OK";

    serializeJson(jsonReturn, ret);
    response->print(ret);
    request->send(response);

    if (Config->GetDebugLevel() >=5) {
      serializeJson(jsonReturn, Serial);
    }
  }
}

//###############################################################
// returns the WebContent
//###############################################################
void handleFiles::GetWebContentConfig(AsyncResponseStream *response) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  response->print("<script language='javascript' type='text/javascript' src='/web/handleFS.js'></script>\n");
  response->print("<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>\n");
  response->print("<table id='maintable' class='editorDemoTable'>\n");
  response->print("<thead>\n");
  response->print("<tr>\n");
  response->print("<td style='width: 200px;'>Dateien</td>\n");
  response->print("<td style='width: 400px;'>Inhalt</td>\n");
  response->print("</tr>\n");
  response->print("</thead>\n");
  response->print("<tbody>\n");
  response->print("<tr>\n");
  response->print("<td style='height:1em;'>\n");
  response->print("<table>\n");
  response->print("<tr>\n");
  response->print("<td style='border: none;'>\n");
  response->print("<b>path:</b>\n");
  response->print("</td>\n");
  response->print("<td style='border: none;'>\n");
  response->print("<div id='path'>{path}</div>\n");
  response->print("<div hidden id='fullpath'>{fullpath}</div>\n");
  response->print("</td>\n");
  response->print("</tr>\n");
  response->print("</table>\n");
  response->print("</td>\n");
  response->print("<td rowspan='2'><textarea id='content' cols='30' rows='10' placeholder='select a file'></textarea></td>\n");
  response->print("</tr>\n");
  response->print("<tr>\n");
  response->print("<td>\n");
  response->print("<template id='NewRow'>\n");
  response->print("<tr>\n");
  response->print("<td style='border: none;' onClick='fetchFile(\"{fullpath}\")'>{file}</td>\n");
  response->print("</tr>\n");
  response->print("</template>\n");
  response->print("<table id='files'></table>\n");
  response->print("</td>\n");
  response->print("</tr>\n");
  response->print("<tr>\n");
  response->print("<td colspan='2'>filename: \n");
  response->print("<input type='text' id='filename' value='{fullpath}' title='name of file to store'/>\n");
  response->print("<input type='button' style='font-size:18px' class='fa' value='&#xf019' onclick='downloadFile()' title='download to pc'/>\n");
  response->print("<input type='button' onclick='uploadFile()' style='font-size:18px' class='fa' value='&#xf0c7' title='store it'/>\n");
  response->print("<span id='response'></span>\n");
  response->print("</td>\n");
  response->print("</tr>\n");
  response->print("</tbody>\n");
  response->print("</table>\n");


}