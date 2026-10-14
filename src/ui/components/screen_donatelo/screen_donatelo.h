#include "bootstrap.h"
//#include "assets/leonardo.h"
#include "services/bluetooth/WatchBleClient.h"

struct ScreenDonateloContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_don;
    WatchBleClient *ble;
};

void screen_donatelo_task_loop(void* param)
{
    ScreenDonateloContext* ctx = static_cast<ScreenDonateloContext*>(param);
    TTGOClass* ttgo = ctx->ttgo;
    bool* screenOn = ctx->screenOn;
    lv_obj_t *tv = ctx->tv;
    lv_obj_t *tile_don = ctx->tile_don;
    short x, y;

    const uint32_t HOLD_MS = 800;
    bool touching = false;
    bool holdSent = false;
    bool ledOn = false;
    unsigned long pressStartedAt = 0;

    WatchBleClient *ble = ctx->ble;

    while (true)
    {
        lv_obj_t *active_tile = lv_tileview_get_tile_act(tv);
        const bool onDon = *screenOn && active_tile == tile_don;

        if (onDon && ttgo->getTouch(x, y))
        {
        const bool onDon = *screenOn && active_tile == tile_don;
            if (!touching)
            {
                touching = true;
                holdSent = false;
                pressStartedAt = millis();
            }
            else if (!holdSent && (millis() - pressStartedAt >= HOLD_MS))
            {
                holdSent = true;

                const bool ok = ledOn
                    ? ble->send(0, 0, 0, 0, LED_EFFECT_OFF)
                    : ble->send(0x90, 0x13, 0xFE, 0xFF, LED_EFFECT_ON);

                if (!ok)
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }
                else
                {
                    ledOn = !ledOn;
                    Serial.println(ledOn ? "Donatelo LED ON" : "Donatelo LED OFF");
                }
            }
        }
        else
        {
            touching = false;
            holdSent = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void screen_donatelo_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_don, WatchBleClient *ble) 
{
    ScreenDonateloContext* ctx = new ScreenDonateloContext{ttgo, screenOn, tv, tile_don, ble};
    xTaskCreate(screen_donatelo_task_loop, "screen_donatelo_task_loop", 2048, ctx, 1, nullptr);
}

void screen_donatelo(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, int col) {

    lv_obj_t * tile_don = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);

    // lv_obj_t * img = lv_img_create(tile_leo);

    // lv_img_set_src(
    //         img,
    //         &leonardo240
    //     );

    // lv_obj_center(img);    

    lv_obj_t * label = lv_label_create(tile_don);
    lv_label_set_text(label, "DONATELO");
    lv_obj_center(label);

    screen_donatelo_sleep_service_task(ttgo, screenOn, tv, tile_don, ble);
}

