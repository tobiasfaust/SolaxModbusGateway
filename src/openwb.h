/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#ifndef OPENWB_H_
#define OPENWB_H_

#include <commonlibs.h>
#include <baseconfig.h>
#include <ArduinoJson.h>

class openwb {
  // openwb mqtt topics
  typedef struct {
    String key;
    String value;
  } openwb_t;

 public:
    openwb();

    /*******************************************************
     * @brief initialize openWB
     *******************************************************/
    void begin(String version);

    /*******************************************************
     * @brief set the needed OpenWB API Version, used from getOpenWbVersions
     *******************************************************/
    void setVersion(String version);

    /*******************************************************
     * get openWB topic from key
     * 
     * @param key: key from openWB JSON
     * @return string: topic
    *******************************************************/
    const String getOpenWbTopic(const String& key);

    /*******************************************************
     * @brief Get all available openWB API Versions
     * 
     * @return std::vector<String>*
     ******************************************************/
    const std::vector<String>* getOpenWbVersions() { return OpenWBVersions; }

    /*******************************************************
     * @brief add a new Mapping for Topic Keys like #key#, 
     * if key still exists, it will be replaced
     * 
     * @param key: key from openWB JSON, without delemite #
     * @param value: value to replace
     * 
     * example: addMapping("key", "battery1");
     * topic definition: /openWB/#key#/value
     * result topic: /openWB/battery1/value
     ******************************************************/
    void addMapping(String key, String value);

    /*******************************************************
     * @brief clear all mappings
     ******************************************************/
    void clearMappings() { OpenWBMappings->clear(); }

 private:
    std::vector<openwb_t>*  OpenWBTopics;         // openWB mqtt topics from JSON
    std::vector<String>*    OpenWBVersions;       // openWB available versions from JSON
    std::vector<openwb_t>*  OpenWBMappings;       // openWB mappings from JSON

    String                  _version;

    void                    LoadOpenWBTopicsFromJson();
    void                    LoadAvailableOpenWbVersions();
};

#endif  // OPENWB_H_
