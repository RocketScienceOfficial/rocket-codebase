#include "hal/uart_driver.h"

void hal_uart_init(uint8_t bus, uint8_t rx, uint8_t tx, uint32_t baudrate)
{
    (void)bus;
    (void)rx;
    (void)tx;
    (void)baudrate;
}

bool hal_uart_is_writable(uint8_t bus)
{
    (void)bus;

    return true;
}

void hal_uart_write(uint8_t bus, const uint8_t *data, size_t size)
{
    (void)bus;
    (void)data;
    (void)size;
}

bool hal_uart_fifo_available(uint8_t bus)
{
    (void)bus;

    return false;
}

size_t hal_uart_read_fifo(uint8_t bus, uint8_t *byte, size_t bufSize)
{
    (void)bus;
    (void)byte;
    (void)bufSize;
}