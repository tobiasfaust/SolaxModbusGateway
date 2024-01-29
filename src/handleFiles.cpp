#include "handleFiles.h"

handleFiles::handleFiles() {

//  Serial.println("Start HandleFiles");
//  StaticJsonDocument<256> doc;
//  JsonArray content = doc.createNestedArray();
//  this->getDirList(&content, "/");
//  serializeJsonPretty(content, Serial);

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
// returns the WebContent
//###############################################################
void handleFiles::GetWebContentConfig(AsyncResponseStream *response) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));

  response->print("<script language='javascript' type='text/javascript' src='/web/handleFS.js'></script>\n");
  response->print("<form id='DataForm'>\n");
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
  response->print("</td>\n");
  response->print("</tr>\n");
  response->print("</table>\n");
  response->print("</td>\n");
  response->print("<td rowspan='2'><textarea id='content' cols='100' rows='20'></textarea></td>\n");
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


  response->print("</tbody>\n");
  response->print("</table>\n");

  response->print("</form>\n\n<br />\n");
  response->print("<form id='jsonform' action='xxxxxxx' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  response->print("  <input type='text' id='json' name='json' />\n");
  response->print("  <input type='submit' value='Speichern' />\n");
  response->print("</form>\n\n");
}