#ifndef WEBPOST_H
#define WEBPOST_H

#include "commonlibs.h" 
#include <PubSubClient.h>
#include "baseconfig.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "modbus.h"


class WEBPOST {

  public:
    WEBPOST(BaseConfig* config);
    void              loop();

  private:
    BaseConfig* _config;
    unsigned long           LastPostTime = 0;
    void PostData();
  
};

extern WEBPOST* webPost;

#endif
