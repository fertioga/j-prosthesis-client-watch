#pragma once
#include "bootstrap.h"
#include "services/ntp/ntp.h"

#define CLOCK_TASK_STACK_SIZE 8192
#define CLOCK_TASK_PRIORITY 1
#define CLOCK_TASK_NAME "CLOCK"

struct Context {
    TTGOClass *ttgo;
    lv_obj_t * parent;
};

void create_clock_exec(void* param)
{

    Context *context = (Context *)param;
    TTGOClass *ttgo = context->ttgo;
    lv_obj_t * parent = context->parent;


    /* HOUR */
    lv_obj_t * containerHour = lv_obj_create(parent);

    lv_obj_set_size(containerHour, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(containerHour, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(containerHour, 6, 0);
    lv_obj_align(containerHour, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(containerHour, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(containerHour, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(containerHour, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(containerHour, 0, LV_PART_MAIN);
    
    lv_obj_t * timeLabel = lv_label_create(containerHour);

    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48, 0); 
    
    /* DATE */
    lv_obj_t * containerDate = lv_obj_create(parent);
    lv_obj_set_size(containerDate, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(containerDate, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(containerDate, 6, 0);
    lv_obj_align(containerDate, LV_ALIGN_CENTER, 0, 80);
    lv_obj_set_style_bg_opa(containerDate, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(containerDate, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(containerDate, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(containerDate, 0, LV_PART_MAIN);

    lv_obj_t * dateLabel = lv_label_create(containerDate);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_14, 0);

    while (true)
    {
        // try to sync RTC every 23 hours, at 00:00:00
        syncRTC(ttgo);

        RTC_Date date = ttgo->rtc->getDateTime();  

        /* SET HOUR */
        char timeStr[6];
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", date.hour, date.minute);       
        lv_label_set_text(timeLabel, timeStr);

        /* SET DATE */
        char dateStr[11];
        snprintf(dateStr, sizeof(dateStr), "%02d/%02d/%04d", date.day, date.month, date.year);
        lv_label_set_text(dateLabel, dateStr);

        vTaskDelay(700 / portTICK_PERIOD_MS); // Update every 700ms to ensure the time changes when the minute changes
    } 

}

void widget_clock_task(TTGOClass *&ttgo, lv_obj_t * parent) {

    Context* ctx = new Context{ttgo, parent};

    xTaskCreate(create_clock_exec, 
            CLOCK_TASK_NAME, 
            CLOCK_TASK_STACK_SIZE, 
            ctx, 
            CLOCK_TASK_PRIORITY, 
            nullptr
    );
}   
