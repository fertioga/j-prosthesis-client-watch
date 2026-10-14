#include "bootstrap.h"
#include "services/bluetooth/WatchBleClient.h"

struct ScreenRafaelContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_raf;
    WatchBleClient *ble;
};

void screen_rafael_task_loop(void* param)
{
    ScreenRafaelContext* ctx = static_cast<ScreenRafaelContext*>(param);
    TTGOClass* ttgo = ctx->ttgo;
    bool* screenOn = ctx->screenOn;
    lv_obj_t *tv = ctx->tv;
    lv_obj_t *tile_raf = ctx->tile_raf;
    short x, y;

    const uint32_t HOLD_MS = 800;
    bool touching = false;
    bool holdSent = false;
    bool ledOnRafael = false;
    unsigned long pressStartedAt = 0;

    WatchBleClient *ble = ctx->ble;

    while (true)
    {
        lv_obj_t *active_tile = lv_tileview_get_tile_act(tv);
        const bool onRaf = *screenOn && active_tile == tile_raf;

        if (onRaf && ttgo->getTouch(x, y))
        {
            if (!touching)
            {
                touching = true;
                holdSent = false;
                pressStartedAt = millis();
            }
            else if (!holdSent && (millis() - pressStartedAt >= HOLD_MS))
            {
                holdSent = true;

                const bool ok = ledOnRafael
                    ? ble->send(0, 0, 0, 0, LED_EFFECT_OFF)
                    : ble->send(0xFF, 0x00, 0x00, 0xFF, LED_EFFECT_ON);

                if (!ok)
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }
                else
                {
                    ledOnRafael = !ledOnRafael;
                    Serial.println(ledOnRafael ? "Rafael LED ON" : "Rafael LED OFF");
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

void screen_rafael_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_raf, WatchBleClient *ble)
{
    ScreenRafaelContext* ctx = new ScreenRafaelContext{ttgo, screenOn, tv, tile_raf, ble};
    xTaskCreate(screen_rafael_task_loop, "screen_rafael_task_loop", 2048, ctx, 1, nullptr);
}

void screen_rafael(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, int col) {
    lv_obj_t * tile_raf = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);

    lv_obj_t * label = lv_label_create(tile_raf);
    lv_label_set_text(label, "RAFAEL");
    lv_obj_center(label);

    screen_rafael_sleep_service_task(ttgo, screenOn, tv, tile_raf, ble);
}
