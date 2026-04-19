#ifndef _BMI088_GYRO_DRIVER_H
#define _BMI088_GYRO_DRIVER_H

/**
 * REF: https://github.com/bolderflight/bmi088-arduino
 * REF: https://github.com/boschsensortec/BMI08x-Sensor-API
 */

#include "bmi088_gyro_driver_defs.h"
#include "bus_utils.h"
#include <lib/maths/vector.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BMI088 gyroscope device
 */
typedef struct
{
    bus_com_device_t device; /** Bus communication device */
    float rangeFactor;       /** Range factor */
} bmi088_gyro_device_t;

/**
 * @brief BMI088 gyroscope ODR and Bandwidth
 */
typedef enum
{
    BMI088_GYRO_ODR_2000_BW_523HZ = BMI088_GYRO_BANDWIDTH_523HZ_CMD,
    BMI088_GYRO_ODR_2000_BW_230HZ = BMI088_GYRO_BANDWIDTH_230HZ_CMD,
    BMI088_GYRO_ODR_1000_BW_116HZ = BMI088_GYRO_BANDWIDTH_116HZ_CMD,
    BMI088_GYRO_ODR_400_BW_47HZ = BMI088_GYRO_BANDWIDTH_47HZ_CMD,
    BMI088_GYRO_ODR_200_BW_23HZ = BMI088_GYRO_BANDWIDTH_23HZ_CMD,
    BMI088_GYRO_ODR_100_BW_12HZ = BMI088_GYRO_BANDWIDTH_12HZ_CMD,
    BMI088_GYRO_ODR_200_BW_64HZ = BMI088_GYRO_BANDWIDTH_64HZ_CMD,
    BMI088_GYRO_ODR_100_BW_32HZ = BMI088_GYRO_BANDWIDTH_32HZ_CMD,
} bmi088_gyro_bandwidth_t;

/**
 * @brief BMI088 gyroscope range
 */
typedef enum
{
    BMI088_GYRO_RANGE_2000DPS = BMI088_GYRO_RANGE_2000DPS_CMD,
    BMI088_GYRO_RANGE_1000DPS = BMI088_GYRO_RANGE_1000DPS_CMD,
    BMI088_GYRO_RANGE_500DPS = BMI088_GYRO_RANGE_500DPS_CMD,
    BMI088_GYRO_RANGE_250DPS = BMI088_GYRO_RANGE_250DPS_CMD,
    BMI088_GYRO_RANGE_125DPS = BMI088_GYRO_RANGE_125DPS_CMD,
} bmi088_gyro_range_t;

/**
 * @brief Initialize BMI088 gyroscope
 *
 * @param device Gyroscope device
 * @param spi SPI
 * @param cs CS
 */
void bmi088_gyro_init_spi(bmi088_gyro_device_t *device, uint8_t spi, uint8_t cs);

/**
 * @brief Initialize BMI088 gyroscope
 *
 * @param device Gyroscope device
 * @param i2c I2C
 * @param sdo1Grounded Is SDO1 pulled to GND
 */
void bmi088_gyro_init_i2c(bmi088_gyro_device_t *device, uint8_t i2c, bool sdo1Grounded);

/**
 * @brief Validate BMI088 gyroscope
 * 
 * @param device Gyroscope device
 * @return true if valid, false otherwise
 */
bool bmi088_gyro_validate(const bmi088_gyro_device_t *device);

/**
 * @brief Set BMI088 gyroscope bandwidth
 *
 * @param device Gyroscope device
 * @param bw Bandwidth
 */
void bmi088_gyro_set_bandwidth(const bmi088_gyro_device_t *device, bmi088_gyro_bandwidth_t bw);

/**
 * @brief Set BMI088 gyroscope range
 *
 * @param device Gyroscope device
 * @param range Range
 */
void bmi088_gyro_set_range(bmi088_gyro_device_t *device, bmi088_gyro_range_t range);

/**
 * @brief Read BMI088 gyroscope data
 *
 * @param device Gyroscope device
 * @param gyro Angular velocity in radians per second
 */
void bmi088_gyro_read(const bmi088_gyro_device_t *device, vec3_t *gyro);

#ifdef __cplusplus
}
#endif

#endif