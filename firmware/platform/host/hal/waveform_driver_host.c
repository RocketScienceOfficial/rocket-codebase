#include "hal/waveform_driver.h"

void hal_waveform_253_init(hal_waveform_channel_t channel, hal_gpio_pin_t pin, uint32_t symbol_rate_hz, uint8_t symbol_bits)
{
    (void)channel;
    (void)pin;
    (void)symbol_rate_hz;
    (void)symbol_bits;
}

void hal_waveform_253_send_blocking(hal_waveform_channel_t channel, const uint32_t *symbols, size_t count)
{
    (void)channel;
    (void)symbols;
    (void)count;
}