#pragma once
#include "bootstrap.h"

#define WATCH_BATTERY_TASK_STACK_SIZE 2048
#define WATCH_BATTERY_TASK_PRIORITY 1
#define WATCH_BATTERY_TASK_NAME "WatchBattery"
#define WATCH_BATTERY_DELAY_MS 60000  // Check battery every 60 seconds

struct WatchBatteryContext {
    TTGOClass *ttgo;
    float *batteryLevel;
};

void watch_battery_exec(void* param) {
    WatchBatteryContext *context = (WatchBatteryContext *)param;
    TTGOClass *ttgo = context->ttgo;
    float *batteryLevel = context->batteryLevel;

    while (true) {
        if (ttgo->power) {
            *batteryLevel = ttgo->power->getBattPercentage();            
        }
        vTaskDelay(WATCH_BATTERY_DELAY_MS / portTICK_PERIOD_MS); 
    }
}

void watch_battery_service_task(TTGOClass *&ttgo, float *&watchBatteryLevel) {

    WatchBatteryContext* ctx = new WatchBatteryContext{ttgo, watchBatteryLevel};

    xTaskCreate(watch_battery_exec, 
            WATCH_BATTERY_TASK_NAME, 
            WATCH_BATTERY_TASK_STACK_SIZE, 
            ctx, 
            WATCH_BATTERY_TASK_PRIORITY, 
            nullptr
    );
}