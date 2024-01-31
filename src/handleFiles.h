#ifndef HANDLEFILES_H
#define HANDLEFILES_H

#include "commonlibs.h"
#include "baseconfig.h"

class handleFiles {
    public:
        handleFiles(AsyncWebServer *server);

        void        GetWebContentConfig(AsyncResponseStream *response);
        void        HandleAjaxRequest(JsonDocument& jsonGet, AsyncResponseStream* response);
        void        handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

    private:
        void        getDirList(JsonArray* json, String path);
};

#endif
