#ifndef HANDLEFILES_H
#define HANDLEFILES_H

#include "commonlibs.h"
#include "baseconfig.h"

class handleFiles {
    public:
        handleFiles();

        void        GetWebContentConfig(AsyncResponseStream *response);
        void        HandleAjaxRequest(JsonDocument& jsonGet, AsyncResponseStream* response);
        
    private:
        void        getDirList(JsonArray* json, String path);
};

#endif
