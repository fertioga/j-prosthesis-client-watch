#include "bootstrap.h"
#include "services/bluetooth/WatchBleClient.h"
#include "ui/led_rgb.h"

static constexpr uint8_t RAF_LED_R = 0xFF;
static constexpr uint8_t RAF_LED_G = 0x00;
static constexpr uint8_t RAF_LED_B = 0x00;
static constexpr LedEffectBle RAF_LED_EFFECT = LED_EFFECT_SOS;

struct ScreenRafaelContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_raf;
    WatchBleClient *ble;
    LedRgb *led;
};

inline void screen_rafael_task_loop(void* param)
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
    unsigned long pressStartedAt = 0;

    WatchBleClient *ble = ctx->ble;
    LedRgb *led = ctx->led;

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

                const bool okRaf = apply_screen_led(ble, led, RAF_LED_R, RAF_LED_G, RAF_LED_B, RAF_LED_EFFECT);

                if (!okRaf)
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }
                else
                {
                    const bool ledOff = led->r == 0 && led->g == 0 && led->b == 0;
                    Serial.println(ledOff ? "Rafael LED OFF" : "Rafael LED ON");
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

inline void screen_rafael_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_raf, WatchBleClient *ble, LedRgb *led)
{
    ScreenRafaelContext* ctx = new ScreenRafaelContext{ttgo, screenOn, tv, tile_raf, ble, led};
    xTaskCreate(screen_rafael_task_loop, "screen_rafael_task_loop", 2048, ctx, 1, nullptr);
}

inline void screen_rafael(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, LedRgb *led, int col) {
    lv_obj_t * tile_raf = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);

    lv_obj_t * label = lv_label_create(tile_raf);
    lv_label_set_text(label, "RAFAEL");
    lv_obj_center(label);

    screen_rafael_sleep_service_task(ttgo, screenOn, tv, tile_raf, ble, led);
}
