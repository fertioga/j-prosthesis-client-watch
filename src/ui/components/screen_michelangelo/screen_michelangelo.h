#include "bootstrap.h"
#include "services/bluetooth/WatchBleClient.h"
#include "ui/led_rgb.h"

static constexpr uint8_t MIC_LED_R = 0xFF;
static constexpr uint8_t MIC_LED_G = 0xA5;
static constexpr uint8_t MIC_LED_B = 0x00;
static constexpr LedEffectBle MIC_LED_EFFECT = LED_EFFECT_STROBO;

struct ScreenMichelangeloContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_mic;
    WatchBleClient *ble;
    LedRgb *led;
};

inline void screen_michelangelo_task_loop(void* param)
{
    ScreenMichelangeloContext* ctx = static_cast<ScreenMichelangeloContext*>(param);
    TTGOClass* ttgo = ctx->ttgo;
    bool* screenOn = ctx->screenOn;
    lv_obj_t *tv = ctx->tv;
    lv_obj_t *tile_mic = ctx->tile_mic;
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
        const bool onMic = *screenOn && active_tile == tile_mic;

        if (onMic && ttgo->getTouch(x, y))
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

                const bool okMic = apply_screen_led(ble, led, MIC_LED_R, MIC_LED_G, MIC_LED_B, MIC_LED_EFFECT);

                if (!okMic)
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }
                else
                {
                    const bool ledOff = led->r == 0 && led->g == 0 && led->b == 0;
                    Serial.println(ledOff ? "Michelangelo LED OFF" : "Michelangelo LED ON");
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

inline void screen_michelangelo_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_mic, WatchBleClient *ble, LedRgb *led)
{
    ScreenMichelangeloContext* ctx = new ScreenMichelangeloContext{ttgo, screenOn, tv, tile_mic, ble, led};
    xTaskCreate(screen_michelangelo_task_loop, "screen_michelangelo_task_loop", 2048, ctx, 1, nullptr);
}

inline void screen_michelangelo(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, LedRgb *led, int col) {
    lv_obj_t * tile_mic = lv_tileview_add_tile(tv, col, 0, LV_DIR_HOR);

    lv_obj_t * label = lv_label_create(tile_mic);
    lv_label_set_text(label, "MICHELANGELO");
    lv_obj_center(label);

    screen_michelangelo_sleep_service_task(ttgo, screenOn, tv, tile_mic, ble, led);
}
