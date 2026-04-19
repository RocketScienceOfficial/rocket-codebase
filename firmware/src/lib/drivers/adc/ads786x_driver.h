#ifndef _ADS786X_DRIVER_H
#define _ADS786X_DRIVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type for ADS786X family
 */
typedef enum
{
    ADS786X_TYPE_6,
    ADS786X_TYPE_7,
    ADS786X_TYPE_8,
} ads786x_type_t;

/**
 * @brief ADS786X device structure
 */
typedef struct
{
    uint8_t spi;
    uint8_t cs;
    uint8_t adcBits;
    float vRef;
} ads786x_device_t;

/**
 * @brief Initialize ADS786X device
 *
 * @param device Device structure
 * @param spi SPI
 * @param cs CS
 * @param type Type
 * @param vRef Reference voltage
 */
void ads786x_init(ads786x_device_t *device, uint8_t spi, uint8_t cs, ads786x_type_t type, float vRef);

/**
 * @brief Read ADS786X voltage
 *
 * @param device Device structure
 * @return Voltage
 */
float ads786x_read(const ads786x_device_t *device);

#ifdef __cplusplus
}
#endif

#endif