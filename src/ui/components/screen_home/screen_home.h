#include "bootstrap.h"
#include "ui/components/widgets/widget_watch_battery.h"

void screen_home(lv_obj_t * tv, TTGOClass *ttgo, int col) 
{
    lv_obj_t * tile1 = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);
    lv_obj_t * label = lv_label_create(tile1);
    lv_label_set_text(label, "HOME");
    lv_obj_center(label);

    widget_watch_battery_service_task(ttgo, tile1);
}   