#include "hal/pwm_driver.h"
#include "driver/ledc.h"

#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY_RAW 1023.0f

typedef struct
{
    uint32_t frequency;
} pwm_timer_state_t;

typedef struct
{
    bool in_use;
    ledc_timer_t timer;
} pwm_channel_state_t;

static pwm_timer_state_t g_timers[LEDC_TIMER_MAX];
static pwm_channel_state_t g_channels[LEDC_CHANNEL_MAX];

bool hal_pwm_init_timer(hal_pwm_timer_t timer, uint32_t frequency)
{
    if (timer >= LEDC_TIMER_MAX)
    {
        return false;
    }

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = (ledc_timer_t)timer,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = frequency,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    bool success = ledc_timer_config(&timer_conf) == ESP_OK;

    g_timers[timer].frequency = frequency;

    return success;
}

void hal_pwm_set_timer_frequency(hal_pwm_timer_t timer, uint32_t frequency)
{
    if (timer >= LEDC_TIMER_MAX || frequency == 0)
    {
        return;
    }

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = (ledc_timer_t)timer,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = frequency,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    g_timers[timer].frequency = frequency;
}

bool hal_pwm_init_channel(hal_pwm_channel_t channel, hal_pwm_timer_t timer, hal_gpio_pin_t pin)
{
    if (channel >= LEDC_CHANNEL_MAX || timer >= LEDC_TIMER_MAX)
    {
        return false;
    }

    ledc_channel_config_t channel_conf = {
        .gpio_num = pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = (ledc_channel_t)channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = (ledc_timer_t)timer,
        .duty = 0,
        .hpoint = 0,
    };

    bool success = ledc_channel_config(&channel_conf) == ESP_OK;

    g_channels[channel].in_use = true;
    g_channels[channel].timer = (ledc_timer_t)timer;

    return success;
}

void hal_pwm_set_channel_duty(hal_pwm_channel_t channel, float dutyCycleUs)
{
    if (channel >= LEDC_CHANNEL_MAX)
    {
        return;
    }

    ledc_timer_t timer = g_channels[channel].timer;
    uint32_t frequency = g_timers[timer].frequency;

    float duty_multiplier = ((float)frequency * PWM_MAX_DUTY_RAW) / 1000000.0f;
    uint32_t raw_duty = (uint32_t)(dutyCycleUs * duty_multiplier);

    if (raw_duty > (uint32_t)PWM_MAX_DUTY_RAW)
    {
        raw_duty = (uint32_t)PWM_MAX_DUTY_RAW;
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel, raw_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel);
}
