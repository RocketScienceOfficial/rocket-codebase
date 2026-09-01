#include "ws2812b_driver.h"

void ws2812b_init(hal_waveform_channel_t channel, hal_gpio_pin_t pin, bool rgbw)
{
    hal_waveform_253_init(channel, pin, 800000, rgbw ? 32 : 24);
}

void ws2812b_set_colors(hal_waveform_channel_t channel, const ws2812b_color_t *colors, size_t count)
{
    hal_waveform_253_send_blocking(channel, (const uint32_t *)colors, count * (sizeof(ws2812b_color_t) / sizeof(uint32_t)));
}