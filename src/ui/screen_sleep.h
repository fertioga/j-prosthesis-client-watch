#pragma once    
#include "bootstrap.h"

struct ScreenSleepContext {
    TTGOClass *ttgo;
    bool *screenOn;
};

void screen_touch_detection(TTGOClass *ttgo, bool *screenOn, unsigned long *lastTouchInteraction)
{
    short x, y;
    const uint32_t DEBOUNCE_MS_TOUCH = 800;

    if (ttgo->getTouch(x, y))
    {
        if (millis() - *lastTouchInteraction > DEBOUNCE_MS_TOUCH)
        {
            if (!*screenOn)
            {
                Serial.println("Screen OFF → Wakeup");
                screen_wakeup(ttgo, screenOn);
            }

            Serial.println("Touch: " + String(x) + "," + String(y));

            *lastTouchInteraction = millis();
        }
    }
}

void screen_auto_sleep(TTGOClass *ttgo, bool *screenOn, unsigned long *lastTouchInteraction)
{
    const uint32_t SCREEN_TIMEOUT = 15000;

    if (*screenOn && (millis() - *lastTouchInteraction > SCREEN_TIMEOUT))
    {
        Serial.println("Sleeping screen...");

        screen_sleep(ttgo, screenOn);

        *lastTouchInteraction = millis();
    }
}

void screen_button_detection(TTGOClass *ttgo, bool *screenOn, unsigned long *lastPress)
{
    const uint32_t DEBOUNCE_MS_BUTTON = 800;

    if (ttgo && ttgo->power)
    {
        // IRQ do PMIC
        if (digitalRead(AXP202_INT) == LOW) // Active LOW
        {
            if (millis() - *lastPress > DEBOUNCE_MS_BUTTON) // Debounce
            {
                *lastPress = millis();

                ttgo->power->readIRQ(); // Read IRQ status from PMIC to clear the interrupt and get the event that triggered it

                if (ttgo->power->isPEKShortPressIRQ()) // SHORT PRESS BUTTON DETECTED
                {
                    if(!*screenOn) { 
                        screen_wakeup(ttgo, screenOn); // WAKEUP SCREEN
                    } else {
                        screen_sleep(ttgo, screenOn); // SLEEP SCREEN
                    }
                }
                ttgo->power->clearIRQ();// Clear the IRQ status in PMIC
            }
        }
    }

}

void screen_sleep_exec(void* param) {

    ScreenSleepContext *ctx = (ScreenSleepContext *)param;
    
    TTGOClass *ttgo = ctx->ttgo;
    bool *screenOn = ctx->screenOn;    

    unsigned long *lastTouchInteraction = new unsigned long(millis());
    unsigned long *lastPress = new unsigned long(0);

    while (true)
    {        
        screen_touch_detection(ttgo, screenOn, *&lastTouchInteraction);

        screen_auto_sleep(ttgo, screenOn, *&lastTouchInteraction);

        screen_button_detection(ttgo, screenOn, *&lastPress);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void screen_sleep_task(TTGOClass *&ttgo, bool *&screenOn) 
{
    ScreenSleepContext* ctx = new ScreenSleepContext{ttgo, screenOn};
    xTaskCreate(screen_sleep_exec, "ScreenSleep", 2048, ctx, 1, nullptr);
}
