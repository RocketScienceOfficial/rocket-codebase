#include "hal/i2c_driver.h"

void hal_i2c_init_bus(hal_i2c_bus_t i2c, hal_gpio_pin_t sda, hal_gpio_pin_t scl, uint32_t baudrate)
{
    (void)i2c;
    (void)sda;
    (void)scl;
    (void)baudrate;
}

bool hal_i2c_transfer(hal_i2c_bus_t bus, uint8_t address, const uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer, size_t rx_size)
{
    (void)bus;
    (void)address;
    (void)tx_buffer;
    (void)tx_size;
    (void)rx_buffer;
    (void)rx_size;

    return true;
}