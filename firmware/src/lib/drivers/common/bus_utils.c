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

    uint8_t txData[2] = {address, value};

    if (!device->useSPI)
    {
        hal_i2c_transfer(device->i2c, device->i2cAddress, txData, sizeof(txData), NULL, 0);
    }
    else
    {

        spi_utils_cs_select(device->cs);
        hal_spi_transfer(device->spi, txData, NULL, sizeof(txData));
        spi_utils_cs_deselect(device->cs);
    }
}

void bus_utils_single_write(const bus_com_device_t *device, uint8_t value)
{
    if (!device->useSPI)
    {
        hal_i2c_transfer(device->i2c, device->i2cAddress, &value, 1, NULL, 0);
    }
    else
    {
        spi_utils_cs_select(device->cs);
        hal_spi_transfer(device->spi, &value, NULL, 1);
        spi_utils_cs_deselect(device->cs);
    }
}

uint8_t bus_utils_read_reg(const bus_com_device_t *device, uint8_t address)
{
    address |= device->readMask;

    uint8_t data;

    if (!device->useSPI)
    {
        hal_i2c_transfer(device->i2c, device->i2cAddress, &address, 1, &data, 1);
    }
    else
    {
        uint8_t txData[2] = {address, 0x00};
        uint8_t rxData[2];

        spi_utils_cs_select(device->cs);
        hal_spi_transfer(device->spi, txData, rxData, sizeof(txData));
        spi_utils_cs_deselect(device->cs);

        data = rxData[1];
    }

    return data;
}

void bus_utils_read_regs(const bus_com_device_t *device, uint8_t address, uint8_t *buffer, size_t count)
{
    address |= device->multipleReadMask;

    if (!device->useSPI)
    {
        hal_i2c_transfer(device->i2c, device->i2cAddress, &address, 1, buffer, count);
    }
    else
    {
        spi_utils_cs_select(device->cs);
        hal_spi_transfer(device->spi, &address, NULL, 1);
        hal_spi_transfer(device->spi, NULL, buffer, count);
        spi_utils_cs_deselect(device->cs);
    }
}

uint8_t bus_utils_single_read(const bus_com_device_t *device)
{
    uint8_t data;

    if (!device->useSPI)
    {
        hal_i2c_transfer(device->i2c, device->i2cAddress, NULL, 0, &data, 1);
    }
    else
    {
        spi_utils_cs_select(device->cs);
        hal_spi_transfer(device->spi, NULL, &data, 1);
        spi_utils_cs_deselect(device->cs);
    }

    return data;
}