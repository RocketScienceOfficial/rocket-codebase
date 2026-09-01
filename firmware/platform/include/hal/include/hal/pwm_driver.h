#ifndef _PWM_DRIVER_H
#define _PWM_DRIVER_H

#include "gpio_driver.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef uint8_t hal_pwm_timer_t;   /** PWM timer/slice definition - the shared frequency domain */
typedef uint8_t hal_pwm_channel_t; /** PWM channel definition - the individual output with its own duty cycle */

/**
 * @brief Initialize a PWM timer at a given frequency. All channels on this timer share this frequency.
 *
 * @param timer Timer to initialize
 * @param frequency Frequency in Hz
 * @return true if success, false if failure
 */
bool hal_pwm_init_timer(hal_pwm_timer_t timer, uint32_t frequency);

/**
 * @brief Change the frequency of a PWM timer. Affects every channel using this timer.
 *
 * @param timer Timer to update
 * @param frequency Frequency in Hz
 */
void hal_pwm_set_timer_frequency(hal_pwm_timer_t timer, uint32_t frequency);

/**
 * @brief Initialize a PWM channel. The channel's timer must already be initialized with hal_pwm_init_timer.
 *
 * @param channel Channel to initialize
 * @param timer Timer to use
 * @param pin GPIO pin to route
 * @return true if success, false if failure
 */
bool hal_pwm_init_channel(hal_pwm_channel_t channel, hal_pwm_timer_t timer, hal_gpio_pin_t pin);

/**
 * @brief Set duty cycle of a PWM channel
 *
 * @param channel Channel to set
 * @param dutyCycleUs Duty cycle in microseconds
 */
void hal_pwm_set_channel_duty(hal_pwm_channel_t channel, float dutyCycleUs);

#ifdef __cplusplus
}
#endif

#endif