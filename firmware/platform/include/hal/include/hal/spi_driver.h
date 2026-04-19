#ifndef _SPI_DRIVER_H
#define _SPI_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SPI
 *
 * @param bus SPI bus
 * @param miso MISO pin
 * @param mosi MOSI pin
 * @param sck SCK pin
 * @param baudrate Baud rate
 */
void hal_spi_init(uint8_t bus, uint8_t miso, uint8_t mosi, uint8_t sck, uint32_t baudrate);

/**
 * @brief Write to SPI with blocking
 *
 * @param bus SPI bus
 * @param data Data pointer to write
 * @param size Size of data
 * @return true if success, false if failure
 */
bool hal_spi_write(uint8_t bus, const uint8_t *data, size_t size);

/**
 * @brief Read from SPI with blocking
 *
 * @param bus SPI bus
 * @param repeatedTXData Buffer to data to write
 * @param destination Data pointer to read from
 * @param size Size of data to receive
 * @return true if success, false if failure
 */
bool hal_spi_read(uint8_t bus, uint8_t repeatedTXData, uint8_t *destination, size_t size);

#ifdef __cplusplus
}
#endif

#endif