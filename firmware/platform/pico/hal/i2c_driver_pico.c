#include "hal/i2c_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

static i2c_inst_t *_get_i2c(uint8_t i2c)
{
    return (i2c == 0 ? i2c0 : i2c1);
}

void hal_i2c_init(uint8_t i2c, uint8_t sda, uint8_t scl, uint32_t baudrate)
{
    i2c_init(_get_i2c(i2c), baudrate);

    hal_gpio_set_pin_function(sda, GPIO_FUNCTION_I2C);
    hal_gpio_set_pin_function(scl, GPIO_FUNCTION_I2C);
    hal_gpio_pull_up_pin(sda);
    hal_gpio_pull_up_pin(scl);
}

bool hal_i2c_write(uint8_t i2c, uint8_t address, const uint8_t *data, size_t size, bool nostop)
{
    return i2c_write_blocking(_get_i2c(i2c), address, data, size, nostop) >= 0;
}

bool hal_i2c_read(uint8_t i2c, uint8_t address, uint8_t *destination, size_t size, bool nostop)
{
    return i2c_read_blocking(_get_i2c(i2c), address, destination, size, nostop) >= 0;
}