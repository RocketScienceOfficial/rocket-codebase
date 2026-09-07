#include "hal/spi_driver.h"

void hal_spi_init_bus(hal_spi_bus_t bus, hal_gpio_pin_t miso, hal_gpio_pin_t mosi, hal_gpio_pin_t sck, uint32_t baudrate)
{
    (void)bus;
    (void)miso;
    (void)mosi;
    (void)sck;
    (void)baudrate;
}

bool hal_spi_transfer(hal_spi_bus_t bus, const uint8_t *outData, uint8_t *inData, size_t size)
{
    (void)bus;
    (void)outData;
    (void)inData;
    (void)size;

    return true;
}