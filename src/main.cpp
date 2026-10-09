#include "bootstrap.h"

#include "ui/screen_main.h"
#include "services/bluetooth/WatchBleClient.h"

TTGOClass *ttgo;

WatchBleClient ble;

bool *screenOn = nullptr; // Pointer to store screen on/off state read by the screen_sleep_service_task
float *watchBatteryLevel = nullptr; // Pointer to store battery level read by the watch_battery_service_task

const uint32_t screenTimeout = 15000;


void setup() {
    Serial.begin(115200);

    screenOn = new bool(true);
    watchBatteryLevel = new float(0.00);

    ble.startConnectionTask();

    screen_main(ttgo, screenOn, watchBatteryLevel, &ble);  
    
    screen_sleep_service_task(ttgo, screenOn);

    delay(500);
    Serial.println("Setup OK");

}

void loop() {

    lv_task_handler();    
    
    delay(30);   
}