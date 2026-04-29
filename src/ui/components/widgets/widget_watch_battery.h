#pragma once
#include "bootstrap.h"
#include "ui/components/widgets/widget_watch_battery.h"

#define WATCH_BATTERY_TASK_STACK_SIZE 2048
#define WATCH_BATTERY_TASK_PRIORITY 1
#define WATCH_BATTERY_TASK_NAME "WatchBattery"
#define WATCH_BATTERY_DELAY_MS 60000  // Check battery every 60 seconds

struct WatchBatteryContext {
    TTGOClass *ttgo;
    lv_obj_t * parent;
};

void watch_battery_exec(void* param) {
    WatchBatteryContext *context = (WatchBatteryContext *)param;
    TTGOClass *ttgo = context->ttgo;
    lv_obj_t * parent = context->parent;

    /* 
     * ***********************************************
     * Battery Widget 
     * ***********************************************
     */
    // Label
    lv_obj_t * batteryLabel = lv_label_create(parent);
    lv_obj_align(batteryLabel, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_label_set_text(batteryLabel, "%");

    // Bar  
    lv_obj_t * batteryBar = lv_bar_create(parent);
    lv_obj_set_size(batteryBar, 40, 12);

    lv_obj_align_to(
        batteryBar,
        batteryLabel,
        LV_ALIGN_OUT_LEFT_MID,
        -10,
        0
    );
    lv_bar_set_range(batteryBar, 0, 100);   
    
    // Update battery level in an infinite loop
    while (true) {
        if (ttgo->power) {
            lv_bar_set_value(batteryBar, (int) ttgo->power->getBattPercentage(), LV_ANIM_OFF);
            lv_label_set_text(batteryLabel, (String((int) ttgo->power->getBattPercentage()) + "%").c_str());
        }

        vTaskDelay(WATCH_BATTERY_DELAY_MS / portTICK_PERIOD_MS); 
    }
}

void widget_watch_battery_service_task(TTGOClass *&ttgo, lv_obj_t * parent) {

    WatchBatteryContext* ctx = new WatchBatteryContext{ttgo, parent};

    xTaskCreate(watch_battery_exec, 
            WATCH_BATTERY_TASK_NAME, 
            WATCH_BATTERY_TASK_STACK_SIZE, 
            ctx, 
            WATCH_BATTERY_TASK_PRIORITY, 
            nullptr
    );
}