#include "hal/pwm_driver.h"

void hal_pwm_init_pin(hal_pwm_config_t *config, uint8_t pin)
{
    (void)config;
    (void)pin;
}

void hal_pwm_set_frequency(hal_pwm_config_t *config, unsigned long frequency)
{
    (void)config;
    (void)frequency;
}

void hal_pwm_set_duty(const hal_pwm_config_t *config, float dutyCycleUs)
{
    (void)config;
    (void)dutyCycleUs;
}