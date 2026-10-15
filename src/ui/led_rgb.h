#pragma once

#include "services/bluetooth/WatchBleClient.h"

struct LedRgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

inline bool apply_screen_led(WatchBleClient *ble, LedRgb *led,
                             uint8_t r, uint8_t g, uint8_t b)
{
    const bool same = led->r == r && led->g == g && led->b == b;

    const bool ok = same
        ? ble->send(0, 0, 0, 0, LED_EFFECT_OFF)
        : ble->send(r, g, b, 0xFF, LED_EFFECT_ON);

    if (!ok)
        return false;

    if (same) {
        led->r = 0;
        led->g = 0;
        led->b = 0;
    } else {
        led->r = r;
        led->g = g;
        led->b = b;
    }
    return true;
}
