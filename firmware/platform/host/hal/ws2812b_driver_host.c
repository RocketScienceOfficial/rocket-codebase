#include "hal/ws2812b_driver.h"

void hal_ws2812b_init(uint8_t pin, bool rgbw)
{
    (void)pin;
    (void)rgbw;
}

void hal_ws2812b_set_colors(const hal_ws2812b_color_t *colors, size_t count)
{
    (void)colors;
    (void)count;
}