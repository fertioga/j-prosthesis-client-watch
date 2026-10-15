#pragma once 
#include "bootstrap.h"
#include "assets/leonardo.h"
#include "services/bluetooth/WatchBleClient.h"
#include "ui/led_rgb.h"

static constexpr uint8_t LEO_LED_R = 0x00;
static constexpr uint8_t LEO_LED_G = 0x00;
static constexpr uint8_t LEO_LED_B = 0xFF;

struct ScreenLeoContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_leo;
    WatchBleClient *ble;
    LedRgb *led;
};

void screen_leo_task_loop(void* param)
{
    ScreenLeoContext* ctx = static_cast<ScreenLeoContext*>(param);
    TTGOClass* ttgo = ctx->ttgo;
    bool* screenOn = ctx->screenOn;
    lv_obj_t *tv = ctx->tv;
    lv_obj_t *tile_leo = ctx->tile_leo;
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
        const bool onLeo = *screenOn && active_tile == tile_leo;

        if (onLeo && ttgo->getTouch(x, y))
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

                const bool okLeo = apply_screen_led(ble, led, LEO_LED_R, LEO_LED_G, LEO_LED_B);

                if (!okLeo)
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }
                else
                {
                    const bool ledOff = led->r == 0 && led->g == 0 && led->b == 0;
                    Serial.println(ledOff ? "Leonardo LED OFF" : "Leonardo LED ON");
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

void screen_leo_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_leo, WatchBleClient *ble, LedRgb *led) 
{
    ScreenLeoContext* ctx = new ScreenLeoContext{ttgo, screenOn, tv, tile_leo, ble, led};
    xTaskCreate(screen_leo_task_loop, "screen_leo_task_loop", 2048, ctx, 1, nullptr);
}

void screen_leonardo(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, LedRgb *led, int col) {
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

    screen_leo_sleep_service_task(ttgo, screenOn, tv, tile_leo, ble, led);
}

