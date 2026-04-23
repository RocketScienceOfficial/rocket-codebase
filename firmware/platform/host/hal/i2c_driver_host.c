#include "hal/i2c_driver.h"

void hal_i2c_init(uint8_t i2c, uint8_t sda, uint8_t scl, uint32_t baudrate)
{
    (void)i2c;
    (void)sda;
    (void)scl;
    (void)baudrate;
}

bool hal_i2c_transfer(uint8_t bus, uint8_t address, const uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer, size_t rx_size)
{
    (void)bus;
    (void)address;
    (void)tx_buffer;
    (void)tx_size;
    (void)rx_buffer;
    (void)rx_size;

    return true;
}