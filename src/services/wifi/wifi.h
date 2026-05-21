#pragma once
#include "bootstrap.h"
#include "config/wifi.h"

void connectToWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void disconnectWiFi() {
    WiFi.disconnect();
    Serial.println("Disconnected from WiFi");
}