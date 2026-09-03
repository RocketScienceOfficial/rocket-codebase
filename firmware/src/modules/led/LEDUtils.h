#pragma once

#define BRIGHTNESS 0.05f

#define LED_COLOR(r, g, b) WS2812B_COLOR_RGB((uint8_t)((r) * BRIGHTNESS), (uint8_t)((g) * BRIGHTNESS), (uint8_t)((b) * BRIGHTNESS))