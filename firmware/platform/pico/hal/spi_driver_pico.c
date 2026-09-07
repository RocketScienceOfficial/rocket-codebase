#include "hal/spi_driver.h"
#include "hal/gpio_driver.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdbool.h>

static spi_inst_t *get_spi(hal_spi_bus_t spi)
{
    return (spi == 0 ? spi0 : spi1);
}

void hal_spi_init_bus(hal_spi_bus_t bus, hal_gpio_pin_t miso, hal_gpio_pin_t mosi, hal_gpio_pin_t sck, uint32_t baudrate)
{
    spi_init(get_spi(bus), baudrate);

    hal_gpio_set_pin_function(miso, HAL_GPIO_FUNCTION_SPI);
    hal_gpio_set_pin_function(mosi, HAL_GPIO_FUNCTION_SPI);
    hal_gpio_set_pin_function(sck, HAL_GPIO_FUNCTION_SPI);
}

bool hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *txData, uint8_t *rxData, size_t size)
{
    if (txData == NULL)
    {
        return spi_read_blocking(get_spi(bus), 0, rxData, size) >= 0;
    }
    else if (rxData == NULL)
    {
        return spi_write_blocking(get_spi(bus), txData, size) >= 0;
    }
    else
    {
        return spi_write_read_blocking(get_spi(bus), txData, rxData, size);
    }
}