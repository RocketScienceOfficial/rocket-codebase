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
 * @brief Transfer data to/from I2C with blocking
 * 
 * @param bus I2C Instance
 * @param address Address of I2C
 * @param tx_buffer Data pointer to write, can be NULL if only reading
 * @param tx_size Size of data to write
 * @param rx_buffer Data pointer to read to, can be NULL if only writing
 * @param rx_size Size of data to receive
 * @return true if success, false if failure
 */
bool hal_i2c_transfer(uint8_t bus, uint8_t address, const uint8_t *tx_buffer, size_t tx_size, uint8_t *rx_buffer, size_t rx_size);

#ifdef __cplusplus
}
#endif

#endif