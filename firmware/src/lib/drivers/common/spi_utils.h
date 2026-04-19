#ifndef _SPI_UTILS_H
#define _SPI_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes SPI CS pin
 * 
 * @param cs CS pin
 */
void spi_utils_cs_init(uint8_t cs);

/**
 * @brief Selects SPI device by setting CS pin low
 * 
 * @param cs CS pin
 */
void spi_utils_cs_select(uint8_t cs);

/**
 * @brief Deselects SPI device by setting CS pin high
 * 
 * @param cs CS pin
 */
void spi_utils_cs_deselect(uint8_t cs);

#ifdef __cplusplus
}
#endif

#endif