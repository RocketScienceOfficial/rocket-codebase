#ifndef _I2C_DRIVER_H
#define _I2C_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2C
 *
 * @param bus I2C Instance
 * @param sda SDA pin
 * @param scl SCL pin
 * @param baudrate Baud rate
 */
void hal_i2c_init(uint8_t bus, uint8_t sda, uint8_t scl, uint32_t baudrate);

/**
 * @brief Write to I2C with blocking
 *
 * @param bus I2C Instance
 * @param address Address of I2C
 * @param data Data pointer to write
 * @param size Size of data
 * @param nostop No stop
 * @return true if success, false if failure
 */
bool hal_i2c_write(uint8_t bus, uint8_t address, const uint8_t *data, size_t size, bool nostop);

/**
 * @brief Read from I2C with blocking
 *
 * @param bus I2C Instance
 * @param address Address of I2C
 * @param destination Data pointer to read to
 * @param size Size of data to receive
 * @param nostop No stop
 * @return true if success, false if failure
 */
bool hal_i2c_read(uint8_t bus, uint8_t address, uint8_t *destination, size_t size, bool nostop);

#ifdef __cplusplus
}
#endif

#endif