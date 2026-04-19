/**
 * REF: https://github.com/raspberrypi/pico-examples/tree/master/pio/ws2812
 */

#include "hal/ws2812b_driver.h"
#include "ws2812b_driver_pico.pio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#define WS2812_FREQ 800000
#define WS2812_PIO pio0
#define WS2812_SM 0

void hal_ws2812b_init(uint8_t pin, bool rgbw)
{
    unsigned int offset = pio_add_program(WS2812_PIO, &ws2812_program);

    ws2812_program_init(WS2812_PIO, WS2812_SM, offset, pin, WS2812_FREQ, rgbw);
}

void hal_ws2812b_set_colors(const hal_ws2812b_color_t *colors, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        pio_sm_put_blocking(WS2812_PIO, WS2812_SM, colors[i] << 8u);
    }
}