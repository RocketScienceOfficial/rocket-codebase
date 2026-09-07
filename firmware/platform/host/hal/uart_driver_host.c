#include "hal/uart_driver.h"

void hal_uart_init_bus(hal_uart_bus_t bus, hal_gpio_pin_t rx, hal_gpio_pin_t tx, uint32_t baudrate)
{
    (void)bus;
    (void)rx;
    (void)tx;
    (void)baudrate;
}

bool hal_uart_is_writable(hal_uart_bus_t bus)
{
    (void)bus;

    return true;
}

void hal_uart_write(hal_uart_bus_t bus, const uint8_t *data, size_t size)
{
    (void)bus;
    (void)data;
    (void)size;
}

bool hal_uart_fifo_available(hal_uart_bus_t bus)
{
    (void)bus;

    return false;
}

size_t hal_uart_read_fifo(hal_uart_bus_t bus, uint8_t *byte, size_t bufSize)
{
    (void)bus;
    (void)byte;
    (void)bufSize;

    return 0;
}