#include "bootstrap.h"

void screen_leonardo(lv_obj_t * tv, int col) {
    lv_obj_t * tile_leo = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);

    lv_obj_t * label = lv_label_create(tile_leo);
    lv_label_set_text(label, "LEONARDO");
    lv_obj_center(label);
}