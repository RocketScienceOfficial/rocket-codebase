#include "hal/adc_driver.h"
#include "hardware/adc.h"
#include <stdbool.h>

#define ADC_VOLTAGE 3.3f
#define ADC_BITS 12
#define ADC_CONVERSION_FACTOR (ADC_VOLTAGE / (1 << ADC_BITS))

static int _convert_pin_to_input(uint8_t pin)
{
    return pin == 26 ? 0 :
           pin == 27 ? 1 :
           pin == 28 ? 2 :
           pin == 29 ? 3 : -1;
}

void hal_adc_init_all(void)
{
    adc_init();
}

void hal_adc_init_pin(uint8_t pin)
{
    adc_gpio_init(pin);
}

float hal_adc_read_voltage(uint8_t pin)
{
    int input = _convert_pin_to_input(pin);

    if (adc_get_selected_input() != input)
    {
        adc_select_input(input);
    }

    return (float)adc_read() * ADC_CONVERSION_FACTOR;
}