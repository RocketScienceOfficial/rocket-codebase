#include "hal/spi_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdbool.h>

static spi_inst_t *_get_spi(uint8_t spi)
{
    return (spi == 0 ? spi0 : spi1);
}

void hal_spi_init(uint8_t bus, uint8_t miso, uint8_t mosi, uint8_t sck, uint32_t baudrate)
{
    spi_init(_get_spi(bus), baudrate);

    hal_gpio_set_pin_function(miso, GPIO_FUNCTION_SPI);
    hal_gpio_set_pin_function(mosi, GPIO_FUNCTION_SPI);
    hal_gpio_set_pin_function(sck, GPIO_FUNCTION_SPI);
}

bool hal_spi_transfer(uint8_t bus, const uint8_t *txData, uint8_t *rxData, size_t size)
{
    if (txData == NULL)
    {
        return spi_read_blocking(_get_spi(bus), 0, rxData, size) >= 0;
    }
    else if (rxData == NULL)
    {
        return spi_write_blocking(_get_spi(bus), txData, size) >= 0;
    }
    else
    {
        return spi_write_read_blocking(_get_spi(bus), txData, rxData, size);
    }
}