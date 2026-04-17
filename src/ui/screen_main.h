#include "bootstrap.h"

#include "ui/components/screen_home/screen_home.h"
#include "ui/components/screen_leonardo/screen_leonardo.h"
#include "ui/components/screen_donatelo/screen_donatelo.h"

void screen_setup() {
    Serial.println("Init Watch...");
    TTGOClass *ttgo;
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
    lv_init();
    ttgo->lvgl_begin();

    Serial.println("LVGL OK");
}

void screen_main() {
    lv_obj_t * tv = lv_tileview_create(lv_scr_act());
    
    lv_obj_set_scroll_dir(tv, LV_DIR_HOR); // only horizontal scroll
    lv_obj_set_scroll_snap_x(tv, LV_SCROLL_SNAP_CENTER); // snap to center of tile
    lv_obj_clear_flag(tv, LV_OBJ_FLAG_SCROLL_ELASTIC); // disable elastic scroll

    /*sequence tiles*/    
    screen_home(tv, 0);
    screen_leonardo(tv, 1);
    screen_donatelo(tv, 2);
}