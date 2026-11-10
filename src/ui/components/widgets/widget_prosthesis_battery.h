#pragma once
#include "bootstrap.h"
#include "services/bluetooth/WatchBleClient.h"

#define PROSTHESIS_BATTERY_TASK_STACK_SIZE 2048
#define PROSTHESIS_BATTERY_TASK_PRIORITY 1
#define PROSTHESIS_BATTERY_TASK_NAME "ProsthesisBattery"
#define PROSTHESIS_BATTERY_DELAY_MS 1000

struct ProsthesisBatteryWidgetContext {
    WatchBleClient *ble;
    lv_obj_t *parent;
    bool *screenOn;
};

void prosthesis_battery_exec(void* param) {
    ProsthesisBatteryWidgetContext *context = (ProsthesisBatteryWidgetContext *)param;
    WatchBleClient *ble = context->ble;
    lv_obj_t *parent = context->parent;
    bool *screenOn = context->screenOn;

    lv_obj_t *label = lv_label_create(parent);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(label, "P --");

    while (true) {
        if (*screenOn) {
            const WatchBleClient::ProsthesisBatteryReading reading = ble->prosthesisBattery();
            char text[12];

            if (reading.known) {
                snprintf(text, sizeof(text), "P %u%%", static_cast<unsigned>(reading.percent));
            } else {
                snprintf(text, sizeof(text), "P --");
            }

            lv_label_set_text(label, text);
        }

        vTaskDelay(PROSTHESIS_BATTERY_DELAY_MS / portTICK_PERIOD_MS);
    }
}

void widget_prosthesis_battery_service_task(WatchBleClient *ble, lv_obj_t *parent, bool *screenOn) {
    ProsthesisBatteryWidgetContext *ctx = new ProsthesisBatteryWidgetContext{ble, parent, screenOn};

    xTaskCreate(prosthesis_battery_exec,
            PROSTHESIS_BATTERY_TASK_NAME,
            PROSTHESIS_BATTERY_TASK_STACK_SIZE,
            ctx,
            PROSTHESIS_BATTERY_TASK_PRIORITY,
            nullptr);
}
