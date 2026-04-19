#include "hal/gpio_driver.h"

void hal_gpio_init_pin(uint8_t pin, hal_gpio_direction_t dir)
{
    (void)pin;
    (void)dir;
}

void hal_gpio_set_pin_state(uint8_t pin, hal_gpio_state_t state)
{
    (void)pin;
    (void)state;
}

hal_gpio_state_t hal_gpio_get_pin_state(uint8_t pin)
{
    (void)pin;

    return GPIO_LOW;
}

void hal_gpio_set_pin_function(uint8_t pin, hal_gpio_function_t function)
{
    (void)pin;
    (void)function;
}

void hal_gpio_pull_up_pin(uint8_t pin)
{
    (void)pin;
}