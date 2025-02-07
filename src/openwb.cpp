/********************************************************
 * Copyright [2024] Tobias Faust <tobias.faust@gmx.net 
 ********************************************************/

#include <openwb.h>

openwb::openwb(): _version("") {
  OpenWBTopics = new std::vector<openwb_t>();
  OpenWBVersions = new std::vector<String>();
  OpenWBMappings = new std::vector<openwb_t>();
}

void openwb::begin(String version) {
  this->_version = version;
  this->LoadAvailableOpenWbVersions();
  this->LoadOpenWBTopicsFromJson();
}

void openwb::setVersion(String version) {
  this->_version = version;
  this->LoadOpenWBTopicsFromJson();
}

void openwb::LoadAvailableOpenWbVersions() {
  File file = LittleFS.open("/misc/openwb.json", "r");
  if (!file) {
    Config->log(1, "Failed to open /misc/openwb.json");
    return;
  }

  OpenWBVersions->clear();

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Config->log(1, "Failed to parse /misc/openwb.json: %s", error.c_str());
    file.close();
    return;
  }

  for (JsonObject v : doc.as<JsonArray>()) {
    OpenWBVersions->push_back(v["version"].as<String>());
    Config->log(3, "OpenWB Version found: %s", v["version"].as<String>().c_str());
  }

  file.close();
}

void openwb::LoadOpenWBTopicsFromJson() {
  File file = LittleFS.open("/misc/openwb.json", "r");
  if (!file) {
    Config->log(1, "Failed to open /misc/openwb.json");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);

  if (error) {
    Config->log(1, "Failed to parse /misc/openwb.json: %s", error.c_str());
    file.close();
    return;
  }

  this->OpenWBTopics->clear();

  for (JsonObject v : doc.as<JsonArray>()) {
    if (v["version"].as<String>() == this->_version) {
      for (JsonObject topic : v["topics"].as<JsonArray>()) {
        for (JsonPair kv : topic) {
          openwb_t t = {};
          t.key = kv.key().c_str();
          t.value = kv.value().as<String>();
          this->OpenWBTopics->push_back(t);

          Config->log(3, "openWB topic loaded: %s", kv.value().as<String>().c_str());
        }
      }
      break;
    }
  }

  file.close();
}

const String openwb::getOpenWbTopic(const String& key) {
  for (uint8_t i = 0; i < this->OpenWBTopics->size(); i++) {
    if (this->OpenWBTopics->at(i).key == key) {
      String topic = this->OpenWBTopics->at(i).value;
      for (uint8_t j = 0; j < this->OpenWBMappings->size(); j++) {
        topic.replace("#" + this->OpenWBMappings->at(j).key + "#", this->OpenWBMappings->at(j).value);
      }
      return topic;
    }
  }
  return "";
}

void openwb::addMapping(String key, String value) {
  for (uint8_t i = 0; i < this->OpenWBMappings->size(); i++) {
    if (this->OpenWBMappings->at(i).key == key) {
      this->OpenWBMappings->at(i).value = value;
      return;
    }
  }

  openwb_t t = {};
  t.key = key;
  t.value = value;
  this->OpenWBMappings->push_back(t);
}
