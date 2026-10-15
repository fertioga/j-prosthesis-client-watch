#pragma once
#include "bootstrap.h"
//#include "assets/leonardo.h"
#include "services/bluetooth/WatchBleClient.h"
#include "ui/led_rgb.h"

static constexpr uint8_t DON_LED_R = 0x90;
static constexpr uint8_t DON_LED_G = 0x13;
static constexpr uint8_t DON_LED_B = 0xFE;

struct ScreenDonateloContext {
    TTGOClass *ttgo;
    bool *screenOn;
    lv_obj_t *tv;
    lv_obj_t *tile_don;
    WatchBleClient *ble;
    LedRgb *led;
};

inline void screen_donatelo_task_loop(void* param)
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
    unsigned long pressStartedAt = 0;

    WatchBleClient *ble = ctx->ble;
    LedRgb *led = ctx->led;

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

                const bool okDon = apply_screen_led(ble, led, DON_LED_R, DON_LED_G, DON_LED_B);

                if (!okDon)
                {
                    Serial.println("Falha ao enviar comando LED via BLE");
                }
                else
                {
                    const bool ledOff = led->r == 0 && led->g == 0 && led->b == 0;
                    Serial.println(ledOff ? "Donatelo LED OFF" : "Donatelo LED ON");
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

inline void screen_donatelo_sleep_service_task(TTGOClass *&ttgo, bool *&screenOn, lv_obj_t *&tv, lv_obj_t *&tile_don, WatchBleClient *ble, LedRgb *led) 
{
    ScreenDonateloContext* ctx = new ScreenDonateloContext{ttgo, screenOn, tv, tile_don, ble, led};
    xTaskCreate(screen_donatelo_task_loop, "screen_donatelo_task_loop", 2048, ctx, 1, nullptr);
}

inline void screen_donatelo(lv_obj_t * tv, TTGOClass * ttgo, bool * screenOn, WatchBleClient * ble, LedRgb *led, int col) {

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

    screen_donatelo_sleep_service_task(ttgo, screenOn, tv, tile_don, ble, led);
}

