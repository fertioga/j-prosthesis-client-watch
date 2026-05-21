#pragma once
#include "bootstrap.h"
#include "config/ntp.h"
#include "services/wifi/wifi.h"

void syncRTC(TTGOClass *ttgo){

    RTC_Date now = ttgo->rtc->getDateTime();

    if(now.hour == hoursToSync && now.minute == minutesToSync && now.second == secondsToSync) {

        connectToWiFi();
        Serial.println("Conectando WiFi...");

        // Configura NTP
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

        struct tm timeinfo;

        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Falha ao obter hora NTP");
            return;
        }

        Serial.println(&timeinfo, "%d/%m/%Y %H:%M:%S");

        // Atualiza RTC da watch
        ttgo->rtc->setDateTime(
            timeinfo.tm_year + 1900,
            timeinfo.tm_mon + 1,
            timeinfo.tm_mday,
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec
        );

        Serial.println("RTC atualizado!");

        disconnectWiFi();
    }
}