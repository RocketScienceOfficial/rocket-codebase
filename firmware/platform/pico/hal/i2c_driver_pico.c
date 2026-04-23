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

bool hal_i2c_transfer(uint8_t bus, uint8_t address, const uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer, size_t rx_size)
{
    bool do_write = tx_buffer != NULL && tx_size > 0;
    bool do_read = rx_buffer != NULL && rx_size > 0;

    if (do_write)
    {
        if (!i2c_write_blocking(_get_i2c(bus), address, tx_buffer, tx_size, do_read))
        {
            return false;
        }
    }

    if (do_read)
    {
        if (!i2c_read_blocking(_get_i2c(bus), address, rx_buffer, rx_size, false))
        {
            return false;
        }
    }

    return true;
}