#include "hal/gpio_driver.h"
#include "pico/stdlib.h"
#include <stdbool.h>

void hal_gpio_init_pin(uint8_t pin, hal_gpio_direction_t dir)
{
    gpio_init(pin);
    gpio_set_dir(pin, dir == GPIO_INPUT ? GPIO_IN : GPIO_OUT);
}

void hal_gpio_set_pin_state(uint8_t pin, hal_gpio_state_t state)
{
    gpio_put(pin, state == GPIO_HIGH ? 1 : 0);
}

hal_gpio_state_t hal_gpio_get_pin_state(uint8_t pin)
{
    return gpio_get(pin) ? GPIO_HIGH : GPIO_LOW;
}

void hal_gpio_set_pin_function(uint8_t pin, hal_gpio_function_t function)
{
    gpio_function_t func = GPIO_FUNC_NULL;

    switch (function)
    {
    case GPIO_FUNCTION_I2C:
        func = GPIO_FUNC_I2C;
        break;
    case GPIO_FUNCTION_SPI:
        func = GPIO_FUNC_SPI;
        break;
    case GPIO_FUNCTION_UART:
        func = GPIO_FUNC_UART;
        break;
    case GPIO_FUNCTION_PWM:
        func = GPIO_FUNC_PWM;
        break;
    default:
        return;
    }

    gpio_set_function(pin, func);
}

void hal_gpio_pull_up_pin(uint8_t pin)
{
    gpio_pull_up(pin);
}