#include "bootstrap.h"
#include "ui/screen_main.h"
#include "services/watch_battery/watch_battery_service.h"

#define RTC_TIME_ZONE   "GMT-3"

TTGOClass *ttgo;

bool *screenOn = nullptr;
float *watchBatteryLevel = nullptr; // Pointer to store battery level read by the watch_battery_service_task

const uint32_t screenTimeout = 15000;

void setup() {
    Serial.begin(115200);
    delay(1000);

    screenOn = new bool(true);
    watchBatteryLevel = new float(0.00);

    screen_main(ttgo);  

    screen_sleep_task(ttgo, screenOn);

    watch_battery_service_task(ttgo, watchBatteryLevel);
    
    delay(2000);
    Serial.println("Setup OK");
}

void loop() {
    lv_task_handler();    

    Serial.println("Battery: " + String(*watchBatteryLevel) + "%");
    delay(1000);
}