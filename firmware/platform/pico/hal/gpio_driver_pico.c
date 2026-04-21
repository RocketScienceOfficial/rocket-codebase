#include "hal/gpio_driver.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdbool.h>

#define PI_PICO_MAX_USER_GPIO (NUM_BANK0_GPIOS)

static irq_handler_t g_callbacks[PI_PICO_MAX_USER_GPIO] = {0};
static uint32_t g_irq_event_masks[PI_PICO_MAX_USER_GPIO] = {0};

static void _irq_handler(uint gpio, uint32_t events)
{
    if (g_irq_event_masks[gpio] == 0)
    {
        return;
    }

    if ((events & g_irq_event_masks[gpio]) && g_callbacks[gpio])
    {
        g_callbacks[gpio]();
    }
}

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

void hal_gpio_attach_interrupt(uint8_t pin, void (*callback)(void), hal_gpio_irq_mode_t mode)
{
    uint32_t pico_mode = 0;

    if (mode & GPIO_IRQ_RISING_EDGE)
    {
        pico_mode |= GPIO_IRQ_EDGE_RISE;
    }
    if (mode & GPIO_IRQ_FALLING_EDGE)
    {
        pico_mode |= GPIO_IRQ_EDGE_FALL;
    }

    g_callbacks[pin] = (irq_handler_t)callback;
    g_irq_event_masks[pin] = pico_mode;

    gpio_set_irq_callback(_irq_handler);
    gpio_set_irq_enabled(pin, pico_mode, true);

    irq_set_enabled(IO_IRQ_BANK0, true);
}

void hal_gpio_detach_interrupt(uint8_t pin)
{
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, false);

    g_callbacks[pin] = NULL;
    g_irq_event_masks[pin] = 0;
}