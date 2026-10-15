#pragma once

#include "services/bluetooth/WatchBleClient.h"

struct LedRgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    LedEffectBle effect;
};

inline bool apply_screen_led(WatchBleClient *ble, LedRgb *led,
                             uint8_t r, uint8_t g, uint8_t b,
                             LedEffectBle effect = LED_EFFECT_ON)
{
    const bool same = led->r == r && led->g == g && led->b == b && led->effect == effect;

    const bool ok = same
        ? ble->send(0, 0, 0, 0, LED_EFFECT_OFF)
        : ble->send(r, g, b, 0xFF, effect);

    if (!ok)
        return false;

    if (same) {
        led->r = 0;
        led->g = 0;
        led->b = 0;
        led->effect = LED_EFFECT_OFF;
    } else {
        led->r = r;
        led->g = g;
        led->b = b;
        led->effect = effect;
    }
    return true;
}
