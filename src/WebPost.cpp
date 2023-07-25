#include "WebPost.h"

WEBPOST::WEBPOST(BaseConfig *config)
{
    this->_config = config;
}

void WEBPOST::PostData()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        const int _JSONDOCSIZE = 1024 * 6;

        http.begin(this->_config->GetWebPostURL().c_str());
        http.addHeader("Content-Type", "application/json");

        // Auth if needed
        if (this->_config->GetWebPostUser().length() > 0 && this->_config->GetWebPostPass().length() > 0) {
            http.setAuthorization(this->_config->GetWebPostUser().c_str(), this->_config->GetWebPostPass().c_str());
        }

        // Prepare JSON document
        DynamicJsonDocument jsonDoc(_JSONDOCSIZE);
        jsonDoc["Type"] = mb->GetInverterType();
        jsonDoc["SN"] = mb->GetInverterSN();
        for (uint16_t i = 0; i < mb->GetInverterLiveDataSize(); i++)
        {
            reg_t reg = mb->GetInverterLiveData(i);
            if (reg.active) {
                jsonDoc["l"][reg.Name]["r"] = reg.RealName;
                jsonDoc["l"][reg.Name]["v"] = reg.value;
                jsonDoc["l"][reg.Name]["u"] = reg.unit;
            }
        }
        String jsonString;
        jsonString.reserve(_JSONDOCSIZE);
        serializeJson(jsonDoc, jsonString);


        int httpResponseCode = http.POST(jsonString); // Send the actual POST request

        if (httpResponseCode > 0)
        {
            String response = http.getString();
            if (this->_config->GetDebugLevel() >=3) {
                Serial.print("Posted Web Data (");
                Serial.print(httpResponseCode);
                Serial.print("): ");
                Serial.println(response);
            }
        }
        else
        {
            if (this->_config->GetDebugLevel() >=0) {
                Serial.print("Error on sending POST: ");
                Serial.println(httpResponseCode);
            }
        }

        http.end();
    }
}

void WEBPOST::loop()
{
    if (this->_config->GetWebPostFrequency() > 0)
    {
        if (millis() - this->LastPostTime > this->_config->GetWebPostFrequency() * 1000)
        {
            this->LastPostTime = millis();

            this->PostData();
        }
    }
}
