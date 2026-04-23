#include "hal/pwm_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/pwm.h"
#include <math.h>
#include <stdbool.h>

typedef struct
{
    unsigned long slice_num;
    unsigned long clock_div;
} pwm_config_t;

static const unsigned long PWM_FREQ_HZ = 125E6;
static const unsigned long PWM_DEFAULT_WRAP = 65535;

static pwm_config_t g_pwm_configs[NUM_BANK0_GPIOS] = {0};

void hal_pwm_init_pin(uint8_t pin)
{
    hal_gpio_set_pin_function(pin, GPIO_FUNCTION_PWM);

    unsigned long slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_enabled(slice_num, true);

    pwm_config_t *config = &g_pwm_configs[pin];
    config->slice_num = slice_num;
    config->clock_div = 1.0f;
}

void hal_pwm_set_frequency(uint8_t pin, unsigned long frequency)
{
    pwm_config_t *config = &g_pwm_configs[pin];

    unsigned long clockDiv = (unsigned long)ceilf((float)PWM_FREQ_HZ / (float)(PWM_DEFAULT_WRAP * frequency));
    unsigned long wrap = (unsigned long)roundf((float)PWM_FREQ_HZ / (float)(clockDiv * frequency));

    pwm_set_clkdiv(config->slice_num, clockDiv);
    pwm_set_wrap(config->slice_num, wrap);

    config->clock_div = clockDiv;
}

void hal_pwm_set_duty(uint8_t pin, float dutyCycleUs)
{
    pwm_config_t *config = &g_pwm_configs[pin];

    unsigned long wrap = (unsigned long)roundf(dutyCycleUs * (PWM_FREQ_HZ / 1e6) / config->clock_div);

    pwm_set_gpio_level(pin, wrap);
}