#pragma once
#include "bootstrap.h"

#include "ui/components/screen_home/screen_home.h"
#include "ui/components/screen_leonardo/screen_leonardo.h"
#include "ui/components/screen_donatelo/screen_donatelo.h"


void screen_main(TTGOClass *&ttgo, bool *&screenOn, float *&watchBatteryLevel) 
{
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
    lv_init();
    ttgo->lvgl_begin();

    lv_obj_t * tv = lv_tileview_create(lv_scr_act());
    
    lv_obj_set_scroll_dir(tv, LV_DIR_HOR); // only horizontal scroll
    lv_obj_set_scroll_snap_x(tv, LV_SCROLL_SNAP_CENTER); // snap to center of tile
    lv_obj_clear_flag(tv, LV_OBJ_FLAG_SCROLL_ELASTIC); // disable elastic scroll

    /*sequence tiles*/    
    screen_home(tv, ttgo, 0);
    screen_leonardo(tv, 1);
    screen_donatelo(tv, 2);
}

void screen_sleep(TTGOClass *&ttgo, bool *&screenOn)
{
    ttgo->displaySleep();
    ttgo->closeBL();
    *screenOn = false;  
    Serial.println("Sleeping...");
    delay(20);
}

void screen_wakeup(TTGOClass *&ttgo, bool *&screenOn)
{
    ttgo->displayWakeup();
    ttgo->openBL();
    *screenOn = true;        
    Serial.println("Wakeup...");
    delay(20); // Delay to avoid screen artifacts on wakeup
}

void userInteraction(TTGOClass *&ttgo, bool *&screenOn)
{
    if (!*screenOn)
    {
        screen_wakeup(ttgo, screenOn);
    }
}