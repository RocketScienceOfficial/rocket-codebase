#include "hal/adc_driver.h"
#include "hardware/adc.h"
#include <stdbool.h>

#define ADC_VOLTAGE 3.3f
#define ADC_BITS 12
#define ADC_CONVERSION_FACTOR (ADC_VOLTAGE / (1 << ADC_BITS))

static uint8_t channel_to_pin(hal_adc_channel_t channel)
{
    return 26 + channel;
}

void hal_adc_init_all(void)
{
    adc_init();
}

void hal_adc_init_channel(hal_adc_channel_t channel)
{
    adc_gpio_init(channel_to_pin(channel));
}

float hal_adc_channel_read_voltage(hal_adc_channel_t channel)
{
    if (adc_get_selected_input() != channel)
    {
        adc_select_input(channel);
    }

    return (float)adc_read() * ADC_CONVERSION_FACTOR;
}