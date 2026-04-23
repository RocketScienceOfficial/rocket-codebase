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
 * @brief Transfer data over SPI with blocking
 * 
 * @param bus SPI bus
 * @param txData Data pointer to write. Can be NULL if only reading.
 * @param rxData Data pointer to read from. Can be NULL if only writing.
 * @param size Size of data to transfer.
 * @return true if success, false if failure
 */
bool hal_spi_transfer(uint8_t bus, const uint8_t *txData, uint8_t *rxData, size_t size);

#ifdef __cplusplus
}
#endif

#endif