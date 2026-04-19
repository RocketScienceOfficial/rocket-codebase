#pragma once

#define OLED_UPDATE_INTERVAL 200
#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22

void oled_init();
void oled_update();