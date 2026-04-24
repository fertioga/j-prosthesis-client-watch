#include "bootstrap.h"
#include "ui/screen_main.h"

#define RTC_TIME_ZONE   "GMT-3"

TTGOClass *ttgo;

bool *screenOn = nullptr;
unsigned long lastPress = 0;
const uint32_t debounceMs = 800;

unsigned long lastInteraction = 0;
const uint32_t screenTimeout = 15000;

void setup() {
    Serial.begin(115200);
    delay(1000);

    screenOn = new bool(true);

    screen_main(ttgo);  
    
    delay(2000);
    Serial.println("Setup OK");
}

void loop() {
    lv_task_handler();

    short x, y;
    unsigned long now = millis();

    /**
     * USER INTERACTION DETECTION TO WAKEUP SCREEN
     * If the screen is touched, we consider it as 
     * an user interaction and reset the screen timeout.
     */
    if (ttgo->getTouch(x, y))
    {
        userInteraction(ttgo, screenOn, lastInteraction);
    }

    /*
    * SLEEP MODE WHEN INACTIVITY
    * If the screen is on and there was no user interaction 
    * for more than the defined timeout, we put the screen to sleep.
    */
    if (*screenOn && (now - lastInteraction > screenTimeout))
    {
        lastInteraction = now;
        screen_sleep(ttgo, screenOn);
        *screenOn = false;
    }
    
    /*
     * POWER BUTTON HANDLER
     * If the power button is pressed, we toggle 
     * the screen state (sleep/wakeup).
     */
    if (ttgo && ttgo->power)
    {
        // IRQ do PMIC
        if (digitalRead(AXP202_INT) == LOW) // Active LOW
        {
            if (now - lastPress > debounceMs) // Debounce
            {
                lastPress = now;

                ttgo->power->readIRQ(); // Read IRQ status from PMIC to clear the interrupt and get the event that triggered it

                if (ttgo->power->isPEKShortPressIRQ()) // SHORT PRESS BUTTON DETECTED
                {
                    if(!*screenOn) { 
                        screen_wakeup(ttgo, screenOn); // WAKEUP SCREEN
                    } else {
                        screen_sleep(ttgo, screenOn); // SLEEP SCREEN
                    }
                }
                ttgo->power->clearIRQ();
            }
        }
    }

    delay(10);
}