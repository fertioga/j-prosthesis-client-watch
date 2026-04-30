#pragma once
#include "bootstrap.h"
#include "ui/components/widgets/widget_watch_battery.h"

#define WATCH_BATTERY_TASK_STACK_SIZE 2048
#define WATCH_BATTERY_TASK_PRIORITY 1
#define WATCH_BATTERY_TASK_NAME "WatchBattery"
#define WATCH_BATTERY_DELAY_MS 3000  // Check battery every 3 seconds

struct WatchBatteryContext {
    TTGOClass *ttgo;
    lv_obj_t * parent;
};

int batteryPercentFromVoltage(float voltage)
{
    if (voltage >= 4.20) return 100;
    if (voltage <= 3.30) return 0;

    return (int)(((voltage - 3.30) / (4.20 - 3.30)) * 100.0);
}

lv_obj_t * create_battery_container(lv_obj_t * parent)
{
    lv_obj_t * container = lv_obj_create(parent);

    lv_obj_remove_style_all(container);

    lv_obj_set_size(container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);

    lv_obj_set_style_pad_column(container, 6, 0);

    lv_obj_align(container, LV_ALIGN_TOP_RIGHT, -10, 10);

    return container;
}

lv_obj_t * create_label_battery(lv_obj_t * parent) {
    lv_obj_t * label = lv_label_create(parent);
    lv_label_set_text(label, "Battery");

    return label;
}

lv_obj_t * create_charge_icon(lv_obj_t * parent)
{
    lv_obj_t * icon = lv_label_create(parent);

    lv_label_set_text(icon, LV_SYMBOL_CHARGE);

    return icon;
}

lv_obj_t * create_bar_battery(lv_obj_t * parent, lv_obj_t * batteryLabel) {
    lv_obj_t * batteryBar = lv_bar_create(parent);

    lv_obj_set_size(batteryBar, 40, 12);

    lv_bar_set_range(batteryBar, 0, 100);

    return batteryBar;
}

void watch_battery_exec(void* param) {
    WatchBatteryContext *context = (WatchBatteryContext *)param;
    TTGOClass *ttgo = context->ttgo;
    lv_obj_t * parent = context->parent;

    /* 
     * ***********************************************
     * Battery Widget 
     * ***********************************************
     */
    lv_obj_t * batteryContainer = create_battery_container(parent);

    lv_obj_t * batteryBar = create_bar_battery(batteryContainer, nullptr);
    lv_obj_t * batteryLabel = create_label_battery(batteryContainer);
    lv_obj_t * chargeIcon = create_charge_icon(batteryContainer);
    
    // Update battery level in an infinite loop
    while (true) {
        if (ttgo->power) {

            float voltage = ttgo->power->getBattVoltage() / 1000.0;

            int battery = batteryPercentFromVoltage(voltage);

            lv_bar_set_value(batteryBar, battery, LV_ANIM_OFF);

            bool charging = ttgo->power->isChargeing();

            lv_label_set_text(
                batteryLabel,
                (String(battery) + "%").c_str()
            );

            if (charging)
            {
                lv_obj_clear_flag(chargeIcon, LV_OBJ_FLAG_HIDDEN);
            }else{               
                lv_obj_add_flag(chargeIcon, LV_OBJ_FLAG_HIDDEN);
            }
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