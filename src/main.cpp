#include "bootstrap.h"
#include "ui/screen_main.h"

#define RTC_TIME_ZONE   "GMT-3"

TTGOClass *ttgo;

bool *screenOn = nullptr;

const uint32_t screenTimeout = 15000;

void setup() {
    Serial.begin(115200);
    delay(1000);

    screenOn = new bool(true);

    screen_main(ttgo);  

    screen_sleep_setup(ttgo, screenOn);
    
    delay(2000);
    Serial.println("Setup OK");
}

void loop() {
    lv_task_handler();
    
    delay(10);
}