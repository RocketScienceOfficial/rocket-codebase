#include "hal/gpio_driver.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"

static void (*g_gpio_callbacks[GPIO_NUM_MAX])(void) = {NULL};
static bool g_isr_service_installed = false;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t pin = (uint32_t)arg;

    if (pin < GPIO_NUM_MAX && g_gpio_callbacks[pin] != NULL)
    {
        g_gpio_callbacks[pin]();
    }
}

void hal_gpio_init_pin(uint8_t pin, hal_gpio_direction_t dir)
{
    gpio_reset_pin((gpio_num_t)pin);
    gpio_set_direction((gpio_num_t)pin, dir == GPIO_OUTPUT ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
}

void hal_gpio_set_pin_state(uint8_t pin, hal_gpio_state_t state)
{
    gpio_set_level((gpio_num_t)pin, state == GPIO_HIGH ? 1 : 0);
}

hal_gpio_state_t hal_gpio_get_pin_state(uint8_t pin)
{
    return gpio_get_level((gpio_num_t)pin) ? GPIO_HIGH : GPIO_LOW;
}

void hal_gpio_set_pin_function(uint8_t pin, hal_gpio_function_t function)
{
}

void hal_gpio_pull_up_pin(uint8_t pin)
{
    gpio_set_pull_mode((gpio_num_t)pin, GPIO_PULLUP_ONLY);
}

void hal_gpio_attach_interrupt(uint8_t pin, void (*callback)(void), hal_gpio_irq_mode_t mode)
{
    if (g_isr_service_installed == false)
    {
        gpio_install_isr_service(0);
        g_isr_service_installed = true;
    }

    gpio_int_type_t intr_type = GPIO_INTR_DISABLE;

    if (mode == GPIO_IRQ_RISING_EDGE)
    {
        intr_type = GPIO_INTR_POSEDGE;
    }
    else if (mode == GPIO_IRQ_FALLING_EDGE)
    {
        intr_type = GPIO_INTR_NEGEDGE;
    }
    else if (mode == (GPIO_IRQ_RISING_EDGE | GPIO_IRQ_FALLING_EDGE))
    {
        intr_type = GPIO_INTR_ANYEDGE;
    }

    g_gpio_callbacks[pin] = callback;

    gpio_set_intr_type((gpio_num_t)pin, intr_type);
    gpio_isr_handler_add((gpio_num_t)pin, gpio_isr_handler, (void *)(uintptr_t)pin);
}

void hal_gpio_detach_interrupt(uint8_t pin)
{
    gpio_isr_handler_remove((gpio_num_t)pin);
    gpio_set_intr_type((gpio_num_t)pin, GPIO_INTR_DISABLE);

    if (pin < 48)
    {
        g_gpio_callbacks[pin] = NULL;
    }
}