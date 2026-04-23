#include "hal/pwm_driver.h"
#include "driver/ledc.h"

#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX_DUTY_RAW 1023.0f

typedef struct
{
    uint8_t pin;
    bool in_use;
    ledc_channel_t channel;
    ledc_timer_t timer;
    float duty_multiplier;
} pwm_config_t;

static pwm_config_t g_configs[LEDC_CHANNEL_MAX];

static pwm_config_t *_get_pwm_config(uint8_t pin)
{
    for (int i = 0; i < LEDC_CHANNEL_MAX; i++)
    {
        if (g_configs[i].in_use && g_configs[i].pin == pin)
        {
            return &g_configs[i];
        }
    }

    return NULL;
}

void hal_pwm_init_pin(uint8_t pin)
{
    if (_get_pwm_config(pin) != NULL)
    {
        return;
    }

    pwm_config_t *state = NULL;

    for (int i = 0; i < LEDC_CHANNEL_MAX; i++)
    {
        if (!g_configs[i].in_use)
        {
            state = &g_configs[i];
            state->pin = pin;
            state->in_use = true;
            state->channel = (ledc_channel_t)i;
            state->timer = (ledc_timer_t)(i % LEDC_TIMER_MAX);
            state->duty_multiplier = 0.0f;
            break;
        }
    }

    if (!state)
    {
        return;
    }

    ledc_channel_config_t channel_conf = {
        .gpio_num = pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = state->channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = state->timer,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&channel_conf);

    hal_pwm_set_frequency(pin, 1000);
}

void hal_pwm_set_frequency(uint8_t pin, unsigned long frequency)
{
    pwm_config_t *state = _get_pwm_config(pin);

    if (!state || frequency == 0)
    {
        return;
    }

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = state->timer,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = frequency,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&timer_conf);

    state->duty_multiplier = ((float)frequency * PWM_MAX_DUTY_RAW) / 1000000.0f;
}

void hal_pwm_set_duty(uint8_t pin, float dutyCycleUs)
{
    pwm_config_t *state = _get_pwm_config(pin);

    if (!state)
    {
        return;
    }

    uint32_t raw_duty = (uint32_t)(dutyCycleUs * state->duty_multiplier);

    if (raw_duty > (uint32_t)PWM_MAX_DUTY_RAW)
    {
        raw_duty = (uint32_t)PWM_MAX_DUTY_RAW;
    }

    ledc_set_duty(LEDC_LOW_SPEED_MODE, state->channel, raw_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, state->channel);
}