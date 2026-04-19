#include "bus_utils.h"
#include "spi_utils.h"
#include <hal/spi_driver.h>
#include <hal/i2c_driver.h>

void bus_utils_init_spi_device(bus_com_device_t *device, uint8_t spi, uint8_t cs, uint8_t readMask, uint8_t multipleReadMask, uint8_t writeMask)
{
    device->useSPI = true;
    device->spi = spi;
    device->cs = cs;
    device->readMask = readMask;
    device->multipleReadMask = multipleReadMask;
    device->writeMask = writeMask;

    spi_utils_cs_init(cs);
}

void bus_utils_init_i2c_device(bus_com_device_t *device, uint8_t i2c, uint8_t address, uint8_t readMask, uint8_t multipleReadMask, uint8_t writeMask)
{
    device->useSPI = false;
    device->i2c = i2c;
    device->i2cAddress = address;
    device->readMask = readMask;
    device->multipleReadMask = multipleReadMask;
    device->writeMask = writeMask;
}

void bus_utils_write_reg_field(const bus_com_device_t *device, uint8_t address, uint8_t length, uint8_t offset, uint8_t value)
{
    uint8_t data = bus_utils_read_reg(device, address);
    uint8_t mask = 0xFF;
    mask >>= offset;
    mask <<= offset;
    mask <<= 8 - offset - length;
    mask >>= 8 - offset - length;

    data &= ~mask;
    data |= (value << offset);

    bus_utils_write_reg(device, address, data);
}

void bus_utils_write_reg(const bus_com_device_t *device, uint8_t address, uint8_t value)
{
    address &= device->writeMask;

    if (!device->useSPI)
    {
        hal_i2c_write(device->i2c, device->i2cAddress, &address, 1, false);
        hal_i2c_read(device->i2c, device->i2cAddress, &value, 1, false);
    }
    else
    {
        spi_utils_cs_select(device->cs);

        hal_spi_write(device->spi, &address, 1);
        hal_spi_write(device->spi, &value, 1);

        spi_utils_cs_deselect(device->cs);
    }
}

void bus_utils_single_write(const bus_com_device_t *device, uint8_t value)
{
    if (!device->useSPI)
    {
        hal_i2c_write(device->i2c, device->i2cAddress, &value, 1, false);
    }
    else
    {
        spi_utils_cs_select(device->cs);

        hal_spi_read(device->spi, 0, &value, 1);

        spi_utils_cs_deselect(device->cs);
    }
}

uint8_t bus_utils_read_reg(const bus_com_device_t *device, uint8_t address)
{
    address |= device->readMask;

    uint8_t data;

    if (!device->useSPI)
    {
        hal_i2c_write(device->i2c, device->i2cAddress, &address, 1, false);
        hal_i2c_read(device->i2c, device->i2cAddress, &data, 1, false);
    }
    else
    {
        spi_utils_cs_select(device->cs);

        hal_spi_write(device->spi, &address, 1);
        hal_spi_read(device->spi, 0, &data, 1);

        spi_utils_cs_deselect(device->cs);
    }

    return data;
}

void bus_utils_read_regs(const bus_com_device_t *device, uint8_t address, uint8_t *buffer, size_t count)
{
    address |= device->multipleReadMask;

    if (!device->useSPI)
    {
        hal_i2c_write(device->i2c, device->i2cAddress, &address, 1, false);
        hal_i2c_read(device->i2c, device->i2cAddress, buffer, count, false);
    }
    else
    {
        spi_utils_cs_select(device->cs);

        hal_spi_write(device->spi, &address, 1);
        hal_spi_read(device->spi, 0, buffer, count);

        spi_utils_cs_deselect(device->cs);
    }
}

uint8_t bus_utils_single_read(const bus_com_device_t *device)
{
    uint8_t data;

    if (!device->useSPI)
    {
        hal_i2c_read(device->i2c, device->i2cAddress, &data, 1, false);
    }
    else
    {
        spi_utils_cs_select(device->cs);

        hal_spi_read(device->spi, 0, &data, 1);

        spi_utils_cs_deselect(device->cs);
    }

    return data;
}