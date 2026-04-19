#ifndef _MS56XX_DRIVER_H
#define _MS56XX_DRIVER_H

#include "ms56xx_driver_defs.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MS56XX OSR
 */
typedef enum
{
    MS56XX_OSR_256 = MS56XX_CMD_CONVERT_D1_OSR_256,
    MS56XX_OSR_512 = MS56XX_CMD_CONVERT_D1_OSR_512,
    MS56XX_OSR_1024 = MS56XX_CMD_CONVERT_D1_OSR_1024,
    MS56XX_OSR_2048 = MS56XX_CMD_CONVERT_D1_OSR_2048,
    MS56XX_OSR_4096 = MS56XX_CMD_CONVERT_D1_OSR_4096,
} ms56xx_osr_t;

/**
 * @brief MS56XX Prom Data (Coefficients)
 */
typedef struct
{
    uint16_t s;
    uint16_t c1;
    uint16_t c2;
    uint16_t c3;
    uint16_t c4;
    uint16_t c5;
    uint16_t c6;
    uint16_t crc;
} ms56xx_prom_data_t;

/**
 * @brief MS56XX device
 */
typedef struct
{
    uint8_t spi;
    uint8_t cs;
    bool version_5611;
    ms56xx_osr_t pressOSR;
    ms56xx_osr_t tempOSR;
    ms56xx_prom_data_t coeffs;
    bool coeffs_valid;
    uint32_t d1;
    uint32_t nextTime;
} ms56xx_device_t;

/**
 * @brief Initalize MS56XX for SPI
 *
 * @param device Device
 * @param spi SPI
 * @param cs CS
 * @param version_5611 True if the device is MS5611, false otherwise
 */
void ms56xx_init_spi(ms56xx_device_t *device, uint8_t spi, uint8_t cs, bool version_5611);

/**
 * @brief Sets OSR
 *
 * @param device Device
 * @param press Pressure OSR
 * @param temp Temperature OSR
 */
void ms56xx_set_osr(ms56xx_device_t *device, ms56xx_osr_t press, ms56xx_osr_t temp);

/**
 * @brief Validates MS56XX device
 *
 * @param device Device
 * @return True if device is valid
 */
bool ms56xx_validate(const ms56xx_device_t *device);

/**
 * @brief Reads values from MS56XX
 *
 * @param device Device
 * @param pressure Pointer to pressure
 * @param temperature Pointer to temperature
 * @return True if values were overwritten
 */
bool ms56xx_read_non_blocking(ms56xx_device_t *device, int *pressure, float *temperature);

#ifdef __cplusplus
}
#endif

#endif