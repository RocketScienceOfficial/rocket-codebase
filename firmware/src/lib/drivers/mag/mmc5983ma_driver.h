#ifndef _MMC5983MA_DRIVER_H
#define _MMC5983MA_DRIVER_H

/**
 * REF: https://github.com/kriswiner/MMC5983MA/tree/master
 * REF: https://www.vectornav.com/resources/inertial-navigation-primer/specifications--and--error-budgets/specs-hsicalibration
 */

#include "mmc5983ma_driver_defs.h"
#include "bus_utils.h"
#include <lib/maths/vector.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MMC5983MA Magnetometer ODR
 */
typedef enum
{
    MMC5983MA_ODR_1HZ = MMC5983MA_CTRL2_CM_FREQ_1HZ,
    MMC5983MA_ODR_10HZ = MMC5983MA_CTRL2_CM_FREQ_10HZ,
    MMC5983MA_ODR_20HZ = MMC5983MA_CTRL2_CM_FREQ_20HZ,
    MMC5983MA_ODR_50HZ = MMC5983MA_CTRL2_CM_FREQ_50HZ,
    MMC5983MA_ODR_100HZ = MMC5983MA_CTRL2_CM_FREQ_100HZ,
    MMC5983MA_ODR_200HZ = MMC5983MA_CTRL2_CM_FREQ_200HZ,
    MMC5983MA_ODR_1000HZ = MMC5983MA_CTRL2_CM_FREQ_1000HZ,
} mmc5983_odr_t;

/**
 * @brief MMC5983MA Magnetometer device
 */
typedef struct
{
    bus_com_device_t device; /** Bus communication device */
} mmc5983ma_device_t;

/**
 * @brief MMC5983MA Magnetometer initialization using SPI
 *
 * @param device MMC5983MA device
 * @param spi SPI instance
 * @param cs CS pin
 */
void mmc5983ma_init_spi(mmc5983ma_device_t *device, uint8_t spi, uint8_t cs);

/**
 * @brief MMC5983MA Magnetometer initialization using I2C
 *
 * @param device MMC5983MA device
 * @param i2c I2C instance
 */
void mmc5983ma_init_i2c(mmc5983ma_device_t *device, uint8_t i2c);

/**
 * @brief Check if product id is valid
 *
 * @param device MMC5983MA device
 * @return Validity
 */
bool mmc5983ma_validate(const mmc5983ma_device_t *device);

/**
 * @brief Set ODR of magnetometer
 *
 * @param device MMC5983MA device
 * @param odr ODR
 */
void mmc5983ma_set_continuous_mode_odr(const mmc5983ma_device_t *device, mmc5983_odr_t odr);

/**
 * @brief Read data from magnetometer
 *
 * @param device MMC5983MA device
 * @param mag Magnetic field in miliGauss
 */
void mmc5983ma_read(const mmc5983ma_device_t *device, vec3_t *mag);

#ifdef __cplusplus
}
#endif

#endif