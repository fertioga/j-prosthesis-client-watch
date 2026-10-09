#include "bootstrap.h"
#include "assets/leonardo.h"
#include "services/bluetooth/WatchBleClient.h"

struct ScreenLeoContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_leo;
    WatchBleClient *ble;
};

void screen_leo_task_loop(void* param)
{
    ScreenLeoContext* ctx = static_cast<ScreenLeoContext*>(param);
    TTGOClass* ttgo = ctx->ttgo;
    bool* screenOn = ctx->screenOn;
    lv_obj_t *tv = ctx->tv;
    lv_obj_t *tile_leo = ctx->tile_leo;
    short x, y;

    const uint32_t DEBOUNCE_MS_TOUCH = 800;
    unsigned long *lastTouchInteraction = new unsigned long(millis());

    WatchBleClient *ble = ctx->ble;

    while (true)
    {        
        if (ttgo->getTouch(x, y))
        {
            lv_obj_t * active_tile = lv_tileview_get_tile_act(tv) ;

            if (
                *screenOn && 
                active_tile == tile_leo && 
                (millis() - *lastTouchInteraction > DEBOUNCE_MS_TOUCH)
                )
            {

                Serial.println("apertou tela Leonardo");

                if (!ble->send(0xFF, 0x00, 0x00, 0xFF, LED_EFFECT_ON))
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }

                *lastTouchInteraction = millis();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void screen_leo_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_leo, WatchBleClient *ble) 
{
    ScreenLeoContext* ctx = new ScreenLeoContext{ttgo, screenOn, tv, tile_leo, ble};
    xTaskCreate(screen_leo_task_loop, "screen_leo_task_loop", 2048, ctx, 1, nullptr);
}

void screen_leonardo(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, int col) {
    lv_obj_t * tile_leo = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);
    
    // lv_obj_t * label = lv_label_create(tile_leo);
    // lv_label_set_text(label, "LEONARDO");
    // lv_obj_center(label);

    lv_obj_t * img = lv_img_create(tile_leo);

    lv_img_set_src(
            img,
            &leonardo240
        );

    lv_obj_center(img);    

    screen_leo_sleep_service_task(ttgo, screenOn, tv, tile_leo, ble);
}

