#ifndef _PWM_DRIVER_H
#define _PWM_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize PWM on pin
 *
 * @param pin Pin to intialize PWM on
 */
void hal_pwm_init_pin(uint8_t pin);

/**
 * @brief Sets frequency of PWM
 *
 * @param pin PWM pin
 * @param frequency Frequency in Hz of PWM
 */
void hal_pwm_set_frequency(uint8_t pin, unsigned long frequency);

/**
 * @brief Set duty cycle of PWM
 *
 * @param pin PWM pin
 * @param dutyCycleUs Duty cycle in microseconds
 */
void hal_pwm_set_duty(uint8_t pin, float dutyCycleUs);

#ifdef __cplusplus
}
#endif

#endif