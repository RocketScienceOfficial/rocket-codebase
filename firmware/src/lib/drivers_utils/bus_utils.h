#ifndef _BUS_UTILS_H
#define _BUS_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bus communication device.
 */
typedef struct
{
    bool useSPI;
    uint8_t spi;
    uint8_t cs;
    uint8_t i2c;
    uint8_t i2cAddress;
    uint8_t readMask;
    uint8_t multipleReadMask;
    uint8_t writeMask;
} bus_com_device_t;

/**
 * @brief Initializes SPI bus communication device
 * 
 * @param device Bus communication device to initialize
 * @param spi SPI peripheral number
 * @param cs Chip select pin number
 * @param readMask Mask to apply to register address for read operations
 * @param multipleReadMask Mask to apply to register address for multiple read operations
 * @param writeMask Mask to apply to register address for write operations
 */
void bus_utils_init_spi_device(bus_com_device_t *device, uint8_t spi, uint8_t cs, uint8_t readMask, uint8_t multipleReadMask, uint8_t writeMask);

/**
 * @brief Initializes I2C bus communication device
 * 
 * @param device Bus communication device to initialize
 * @param i2c I2C peripheral number
 * @param address I2C device address
 * @param readMask Mask to apply to register address for read operations
 * @param multipleReadMask Mask to apply to register address for multiple read operations
 * @param writeMask Mask to apply to register address for write operations
 */
void bus_utils_init_i2c_device(bus_com_device_t *device, uint8_t i2c, uint8_t address, uint8_t readMask, uint8_t multipleReadMask, uint8_t writeMask);

/**
 * @brief Writes register field
 *
 * @param device Bus device
 * @param address Register address
 * @param length Field length
 * @param offset Field offset
 * @param value Field value
 */
void bus_utils_write_reg_field(const bus_com_device_t *device, uint8_t address, uint8_t length, uint8_t offset, uint8_t value);

/**
 * @brief Writes single register
 *
 * @param device Bus device
 * @param address Register address
 * @param value Data to write
 */
void bus_utils_write_reg(const bus_com_device_t *device, uint8_t address, uint8_t value);

/**
 * @brief Performs a single write operation
 *
 * @param device Bus device
 * @param value Value to be written
 */
void bus_utils_single_write(const bus_com_device_t *device, uint8_t value);

/**
 * @brief Reads single register
 *
 * @param device Bus device
 * @param address Register address
 * @return Register value
 */
uint8_t bus_utils_read_reg(const bus_com_device_t *device, uint8_t address);

/**
 * @brief Reads multiple registers
 *
 * @param device Bus device
 * @param address Starting register address
 * @param buffer Buffer to store data
 * @param count Count of registers to read
 */
void bus_utils_read_regs(const bus_com_device_t *device, uint8_t address, uint8_t *buffer, size_t count);

/**
 * @brief Performs a single read operation
 *
 * @param device Bus device
 * @return Read value
 */
uint8_t bus_utils_single_read(const bus_com_device_t *device);

#ifdef __cplusplus
}
#endif

#endif