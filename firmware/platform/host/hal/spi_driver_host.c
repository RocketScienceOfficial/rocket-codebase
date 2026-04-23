#include "hal/spi_driver.h"

void hal_spi_init(uint8_t bus, uint8_t miso, uint8_t mosi, uint8_t sck, uint32_t baudrate)
{
    (void)bus;
    (void)miso;
    (void)mosi;
    (void)sck;
    (void)baudrate;
}

bool hal_spi_transfer(uint8_t bus, const uint8_t *outData, uint8_t *inData, size_t size)
{
    (void)bus;
    (void)outData;
    (void)inData;
    (void)size;

    return true;
}