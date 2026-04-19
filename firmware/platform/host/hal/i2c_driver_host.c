#include "hal/i2c_driver.h"

void hal_i2c_init(uint8_t i2c, uint8_t sda, uint8_t scl, uint32_t baudrate)
{
    (void)i2c;
    (void)sda;
    (void)scl;
    (void)baudrate;
}

bool hal_i2c_write(uint8_t i2c, uint8_t address, const uint8_t *data, size_t size, bool nostop)
{
    (void)i2c;
    (void)address;
    (void)data;
    (void)size;
    (void)nostop;

    return true;
}

bool hal_i2c_read(uint8_t i2c, uint8_t address, uint8_t *destination, size_t size, bool nostop)
{
    (void)i2c;
    (void)address;
    (void)destination;
    (void)size;
    (void)nostop;

    return true;
}