#include "hal/pwm_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/pwm.h"
#include <math.h>
#include <stdbool.h>

#define NUM_CHANNELS_PER_SLICE 2

typedef struct
{
    unsigned long clock_div;
    hal_gpio_pin_t channels_pins[NUM_CHANNELS_PER_SLICE];
} pwm_timer_state_t;

static const unsigned long PWM_FREQ_HZ = 125E6;
static const unsigned long PWM_DEFAULT_WRAP = 65535;

static pwm_timer_state_t g_pwm_timers[NUM_PWM_SLICES] = {0};

static void compute_clkdiv_wrap(uint32_t frequency, unsigned long *clockDiv, unsigned long *wrap)
{
    *clockDiv = (unsigned long)ceilf((float)PWM_FREQ_HZ / (float)(PWM_DEFAULT_WRAP * frequency));
    *wrap = (unsigned long)roundf((float)PWM_FREQ_HZ / (float)(*clockDiv * frequency));
}

bool hal_pwm_init_timer(hal_pwm_timer_t timer, uint32_t frequency)
{
    if (timer >= NUM_PWM_SLICES)
    {
        return false;
    }

    unsigned long clockDiv;
    unsigned long wrap;
    compute_clkdiv_wrap(frequency, &clockDiv, &wrap);

    pwm_set_clkdiv(timer, clockDiv);
    pwm_set_wrap(timer, wrap);
    pwm_set_enabled(timer, true);

    g_pwm_timers[timer].clock_div = clockDiv;

    return true;
}

void hal_pwm_set_timer_frequency(hal_pwm_timer_t timer, uint32_t frequency)
{
    if (timer >= NUM_PWM_SLICES)
    {
        return;
    }

    unsigned long clockDiv;
    unsigned long wrap;
    compute_clkdiv_wrap(frequency, &clockDiv, &wrap);

    pwm_set_clkdiv(timer, clockDiv);
    pwm_set_wrap(timer, wrap);

    g_pwm_timers[timer].clock_div = clockDiv;
}

bool hal_pwm_init_channel(hal_pwm_channel_t channel, hal_pwm_timer_t timer, hal_gpio_pin_t pin)
{
    if (channel >= NUM_CHANNELS_PER_SLICE || timer >= NUM_PWM_SLICES)
    {
        return false;
    }

    if (pwm_gpio_to_slice_num(pin) != timer)
    {
        return false;
    }

    hal_gpio_set_pin_function(pin, HAL_GPIO_FUNCTION_PWM);

    g_pwm_timers[timer].channels_pins[channel] = pin;

    return true;
}

void hal_pwm_set_channel_duty(hal_pwm_channel_t channel, hal_pwm_timer_t timer, float dutyCycleUs)
{
    if (channel >= NUM_CHANNELS_PER_SLICE)
    {
        return;
    }
    
    hal_gpio_pin_t pin = g_pwm_timers[timer].channels_pins[channel];
    unsigned long clock_div = g_pwm_timers[timer].clock_div;

    unsigned long wrap = (unsigned long)roundf(dutyCycleUs * (PWM_FREQ_HZ / 1e6) / clock_div);

    pwm_set_gpio_level(pin, wrap);
}
