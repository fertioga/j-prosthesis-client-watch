#include "bootstrap.h"
#include "ui/screen_main.h"
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Init Watch...");

    screen_setup();

    Serial.println("LVGL OK");

    screen_main();

}

void loop() {
    lv_task_handler();
    delay(10);
}