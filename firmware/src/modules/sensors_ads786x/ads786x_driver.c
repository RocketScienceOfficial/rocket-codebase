#include "ads786x_driver.h"
#include "spi_utils.h"
#include <hal/spi_driver.h>
#include <hal/time_driver.h>
#include <lib/debug/sys_assert.h>

static uint8_t ads786x_get_adc_bits(ads786x_type_t type)
{
    switch (type)
    {
    case ADS786X_TYPE_6:
        return 12;
    case ADS786X_TYPE_7:
        return 10;
    case ADS786X_TYPE_8:
        return 8;
    default:
        return -1;
    }
}

void ads786x_init(ads786x_device_t *device, uint8_t spi, uint8_t cs, ads786x_type_t type, float vRef)
{
    SYS_ASSERT(device != NULL);

    device->spi = spi;
    device->cs = cs;
    device->adcBits = ads786x_get_adc_bits(type);
    device->vRef = vRef;

    SYS_ASSERT(device->adcBits != -1);

    spi_utils_cs_init(cs);
}

float ads786x_read(const ads786x_device_t *device)
{
    SYS_ASSERT(device != NULL);
    
    spi_utils_cs_select(device->cs);

    hal_time_sleep_us(25);

    uint8_t buffer[2];
    hal_spi_transfer(device->spi, NULL, buffer, sizeof(buffer));

    spi_utils_cs_deselect(device->cs);

    uint16_t data = ((buffer[0] << 8) | buffer[1]) >> (12 - device->adcBits);
    float result = device->vRef * (float)data / (1 << device->adcBits);

    return result;
}