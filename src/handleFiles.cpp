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
  JsonDocument doc;
  JsonObject jsonRoot = doc.to<JsonObject>();

  jsonRoot["path"] = path;
  JsonArray content = jsonRoot["content"].to<JsonArray>();

  File FSroot = LittleFS.open(path);
  File file = FSroot.openNextFile();

  while (file) {
    JsonDocument doc1;
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
  String subaction = "";
  if (jsonGet["subaction"])  {subaction  = jsonGet["subaction"].as<String>();}

  if (Config->GetDebugLevel() >= 3) {
    dbg.printf("handle Ajax Request in handleFiles.cpp: %s\n", subaction.c_str());
  }

  if (subaction == "listDir") {
    JsonDocument doc;
    JsonArray content = doc.add<JsonArray>();
    
    this->getDirList(&content, "/");
    String ret("");
    serializeJson(content, ret);
    if (Config->GetDebugLevel() >= 5) {
      serializeJsonPretty(content, dbg);
      dbg.println();
    }
    response->print(ret);   
  } else if (subaction == "deleteFile") {
    String filename(""), ret("");
    JsonDocument jsonReturn;

    if (Config->GetDebugLevel() >=3) {
      dbg.printf("Request to delete file %s", filename.c_str());
    }
    if (jsonGet["filename"])  {filename  = jsonGet["filename"].as<String>();}
    
    if (LittleFS.remove(filename)) { 
      jsonReturn["response_status"] = 1;
      jsonReturn["response_text"] = "deletion successful";
    } else {
      jsonReturn["response_status"] = 0;
      jsonReturn["response_text"] = "deletion failed";
    }
    if (Config->GetDebugLevel() >=3) {
      serializeJson(jsonReturn, Serial);dbg.println();
    } 
    serializeJson(jsonReturn, ret);
    response->print(ret);
  }
}

//###############################################################
// store a file at Filesystem
//###############################################################
void handleFiles::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
  if (Config->GetDebugLevel() >=5) {
    dbg.printf("Client: %s %s\n", request->client()->remoteIP().toString().c_str(), request->url().c_str());;
  }

  if (!index) {
    // open the file on first call and store the file handle in the request object
    request->_tempFile = LittleFS.open(filename, "w");
    if (Config->GetDebugLevel() >=5) {
      dbg.printf("Upload Start: %s\n", filename.c_str());
    }
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    if (Config->GetDebugLevel() >=5) {
      dbg.printf("Writing file: %s ,index=%d len=%d bytes, FreeMem: %d\n", filename.c_str(), index, len, ESP.getFreeHeap());
    }
  }

  if (final) {
    // close the file handle as the upload is now done
    request->_tempFile.close();
    if (Config->GetDebugLevel() >=3) {
      dbg.printf("Upload Complete: %s ,size: %d Bytes\n", filename.c_str(), (index + len));
    }

    AsyncResponseStream *response = request->beginResponseStream("text/json");
    response->addHeader("Server","ESP Async Web Server");

    JsonDocument jsonReturn;
    String ret;

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
