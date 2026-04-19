#ifndef _PWM_DRIVER_H
#define _PWM_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief PWM protocol configuration
 */
typedef struct
{
    uint8_t pin;
    unsigned long clockDiv;
} hal_pwm_config_t;

/**
 * @brief Initialize PWM on pin
 *
 * @param config PWM configuration
 * @param pin Pin to intialize
 */
void hal_pwm_init_pin(hal_pwm_config_t *config, uint8_t pin);

/**
 * @brief Sets frequency of PWM
 *
 * @param config PWM configuration
 * @param frequency Frequency in Hz of PWM
 */
void hal_pwm_set_frequency(hal_pwm_config_t *config, unsigned long frequency);

/**
 * @brief Set duty cycle of PWM
 *
 * @param config PWM configuration
 * @param dutyCycleUs Duty cycle in microseconds
 */
void hal_pwm_set_duty(const hal_pwm_config_t *config, float dutyCycleUs);

#ifdef __cplusplus
}
#endif

#endif