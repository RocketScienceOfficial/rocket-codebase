#include "hal/gpio_driver.h"

void hal_gpio_init_pin(hal_gpio_pin_t pin, hal_gpio_direction_t dir)
{
    (void)pin;
    (void)dir;
}

void hal_gpio_set_pin_state(hal_gpio_pin_t pin, hal_gpio_state_t state)
{
    (void)pin;
    (void)state;
}

hal_gpio_state_t hal_gpio_get_pin_state(hal_gpio_pin_t pin)
{
    (void)pin;

    return HAL_GPIO_LOW;
}

void hal_gpio_set_pin_function(hal_gpio_pin_t pin, hal_gpio_function_t function)
{
    (void)pin;
    (void)function;
}

void hal_gpio_pull_up_pin(hal_gpio_pin_t pin)
{
    (void)pin;
}

void hal_gpio_attach_interrupt(hal_gpio_pin_t pin, void (*callback)(void), hal_gpio_irq_mode_t mode)
{
    (void)pin;
    (void)callback;
    (void)mode;
}

void hal_gpio_detach_interrupt(hal_gpio_pin_t pin)
{
    (void)pin;
}