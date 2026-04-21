#ifndef _GPIO_DRIVER_H
#define _GPIO_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type for GPIO pin mode
 */
typedef enum
{
    GPIO_INPUT = 0,
    GPIO_OUTPUT = 1
} hal_gpio_direction_t;

/**
 * @brief Type for GPIO pin state
 */
typedef enum
{
    GPIO_LOW = 0,
    GPIO_HIGH = 1
} hal_gpio_state_t;

/**
 * @brief Type for GPIO pin interrupt mode
 */
typedef enum
{
    GPIO_IRQ_RISING_EDGE = (1 << 0),
    GPIO_IRQ_FALLING_EDGE = (1 << 1),
} hal_gpio_irq_mode_t;

/**
 * @brief Type for GPIO pin function
 */
typedef enum
{
    GPIO_FUNCTION_I2C = 0,
    GPIO_FUNCTION_SPI = 1,
    GPIO_FUNCTION_UART = 2,
    GPIO_FUNCTION_PWM = 3,
} hal_gpio_function_t;

/**
 * @brief Initialize GPIO pin
 *
 * @param pin Pin to initialize
 * @param dir Direction of pin
 */
void hal_gpio_init_pin(uint8_t pin, hal_gpio_direction_t dir);

/**
 * @brief Set state of GPIO pin
 *
 * @param pin Pin to set state
 * @param state State to set
 */
void hal_gpio_set_pin_state(uint8_t pin, hal_gpio_state_t state);

/**
 * @brief Get state of GPIO pin
 *
 * @param pin Pin to get state of
 * @return State of pin
 */
hal_gpio_state_t hal_gpio_get_pin_state(uint8_t pin);

/**
 * @brief Set function of GPIO pin
 *
 * @param pin Pin to set function
 * @param function Function to set
 */
void hal_gpio_set_pin_function(uint8_t pin, hal_gpio_function_t function);

/**
 * @brief Pull GPIO pin up
 *
 * @param pin Pin to pull up
 */
void hal_gpio_pull_up_pin(uint8_t pin);

/**
 * @brief Attach interrupt to GPIO pin
 * 
 * @param pin Pin to attach interrupt
 * @param callback Callback function to call when interrupt is triggered
 * @param mode Interrupt mode (rising edge, falling edge, or both)
 */
void hal_gpio_attach_interrupt(uint8_t pin, void (*callback)(void), hal_gpio_irq_mode_t mode);

/**
 * @brief Detach interrupt from GPIO pin
 * 
 * @param pin Pin to detach interrupt from
 */
void hal_gpio_detach_interrupt(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif