#include "hal/pwm_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/pwm.h"
#include <math.h>
#include <stdbool.h>

static const unsigned long PWM_FREQ_HZ = 125E6;
static const unsigned long PWM_DEFAULT_WRAP = 65535;

void hal_pwm_init_pin(hal_pwm_config_t *config, uint8_t pin)
{
    config->pin = pin;

    hal_gpio_set_pin_function(pin, GPIO_FUNCTION_PWM);

    unsigned long slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_enabled(slice_num, true);
}

void hal_pwm_set_frequency(hal_pwm_config_t *config, unsigned long frequency)
{
    unsigned long clockDiv = (unsigned long)ceilf((float)PWM_FREQ_HZ / (float)(PWM_DEFAULT_WRAP * frequency));
    unsigned long wrap = (unsigned long)roundf((float)PWM_FREQ_HZ / (float)(clockDiv * frequency));
    unsigned long slice_num = pwm_gpio_to_slice_num(config->pin);

    pwm_set_clkdiv(slice_num, clockDiv);
    pwm_set_wrap(slice_num, wrap);

    config->clockDiv = clockDiv;
}

void hal_pwm_set_duty(const hal_pwm_config_t *config, float dutyCycleUs)
{
    unsigned long wrap = (unsigned long)roundf(dutyCycleUs * (PWM_FREQ_HZ / 1e6) / config->clockDiv);

    pwm_set_gpio_level(config->pin, wrap);
}