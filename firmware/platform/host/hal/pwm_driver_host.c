#include "hal/pwm_driver.h"

bool hal_pwm_init_timer(hal_pwm_timer_t timer, uint32_t frequency)
{
    (void)timer;
    (void)frequency;

    return true;
}

void hal_pwm_set_timer_frequency(hal_pwm_timer_t timer, uint32_t frequency)
{
    (void)timer;
    (void)frequency;
}

bool hal_pwm_init_channel(hal_pwm_channel_t channel, hal_pwm_timer_t timer, hal_gpio_pin_t pin)
{
    (void)channel;
    (void)timer;
    (void)pin;

    return true;
}

void hal_pwm_set_channel_duty(hal_pwm_channel_t channel, hal_pwm_timer_t timer, float dutyCycleUs)
{
    (void)channel;
    (void)timer;
    (void)dutyCycleUs;
}
