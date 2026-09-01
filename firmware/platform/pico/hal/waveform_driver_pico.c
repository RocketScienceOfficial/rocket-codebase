/**
 * REF: https://github.com/raspberrypi/pico-examples/tree/master/pio/ws2812
 */

#include "hal/waveform_driver.h"
#include "waveform_driver_pico.pio.h"
#include "waveform_utils_pico.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

void hal_waveform_253_init(hal_waveform_channel_t channel, hal_gpio_pin_t pin, uint32_t symbol_rate_hz, uint8_t symbol_bits)
{
    PIO pio = HAL_WAVEFORM_PICO_CHANNEL_GET_PIO(channel);
    uint8_t sm = HAL_WAVEFORM_PICO_CHANNEL_GET_SM(channel);

    unsigned int offset = pio_add_program(pio, &waveform_program);

    waveform_program_init(pio, sm, offset, pin, symbol_rate_hz, symbol_bits);
}

void hal_waveform_253_send_blocking(hal_waveform_channel_t channel, const uint32_t *symbols, size_t count)
{
    PIO pio = HAL_WAVEFORM_PICO_CHANNEL_GET_PIO(channel);
    uint8_t sm = HAL_WAVEFORM_PICO_CHANNEL_GET_SM(channel);

    for (size_t i = 0; i < count; i++)
    {
        pio_sm_put_blocking(pio, sm, symbols[i]);
    }
}