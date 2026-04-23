#include "hal/pwm_driver.h"

void hal_pwm_init_pin(uint8_t pin)
{
    (void)pin;
}

void hal_pwm_set_frequency(uint8_t pin, unsigned long frequency)
{
    (void)pin;
    (void)frequency;
}

void hal_pwm_set_duty(uint8_t pin, float dutyCycleUs)
{
    (void)pin;
    (void)dutyCycleUs;
}