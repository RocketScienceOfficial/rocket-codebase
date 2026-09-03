#ifndef _BMI088_ACC_DRIVER_H
#define _BMI088_ACC_DRIVER_H

/**
 * REF: https://github.com/bolderflight/bmi088-arduino
 * REF: https://github.com/boschsensortec/BMI08x-Sensor-API
 */

#include "bmi088_acc_driver_defs.h"
#include "bus_utils.h"
#include <lib/maths/vector.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BMI088 accelerometer device
 */
typedef struct
{
    bus_com_device_t device; /** Bus communication device */
    float rangeFactor;       /** Range factor */
} bmi088_acc_device_t;

/**
 * @brief BMI088 accelerometer range
 */
typedef enum
{
    BMI088_ACC_RANGE_3G = BMI088_ACC_RANGE_3G_CMD,
    BMI088_ACC_RANGE_6G = BMI088_ACC_RANGE_6G_CMD,
    BMI088_ACC_RANGE_12G = BMI088_ACC_RANGE_12G_CMD,
    BMI088_ACC_RANGE_24G = BMI088_ACC_RANGE_24G_CMD,
} bmi088_acc_range_t;

/**
 * @brief BMI088 accelerometer ODR
 */
typedef enum
{
    BMI088_ACC_ODR_12_5HZ = BMI088_ACC_CONF_ODR_12_5HZ,
    BMI088_ACC_ODR_25HZ = BMI088_ACC_CONF_ODR_25HZ,
    BMI088_ACC_ODR_50HZ = BMI088_ACC_CONF_ODR_50HZ,
    BMI088_ACC_ODR_100HZ = BMI088_ACC_CONF_ODR_100HZ,
    BMI088_ACC_ODR_200HZ = BMI088_ACC_CONF_ODR_200HZ,
    BMI088_ACC_ODR_400HZ = BMI088_ACC_CONF_ODR_400HZ,
    BMI088_ACC_ODR_800HZ = BMI088_ACC_CONF_ODR_800HZ,
    BMI088_ACC_ODR_1600HZ = BMI088_ACC_CONF_ODR_1600HZ,
} bmi088_acc_odr_t;

/**
 * @brief BMI088 accelerometer OSR
 */
typedef enum
{
    BMI088_ACC_OSR_NORMAL = BMI088_ACC_CONF_OSR_NORMAL,
    BMI088_ACC_OSR_2 = BMI088_ACC_CONF_OSR_2,
    BMI088_ACC_OSR_4 = BMI088_ACC_CONF_OSR_4,
} bmi088_acc_osr_t;

/**
 * @brief Initialize BMI088 accelerometer
 *
 * @param device Accelerometer device
 * @param spi SPI
 * @param cs CS
 */
void bmi088_acc_init_spi(bmi088_acc_device_t *device, uint8_t spi, uint8_t cs);

/**
 * @brief Initialize BMI088 accelerometer
 *
 * @param device Accelerometer device
 * @param i2c I2C
 * @param sdo1Grounded Is SDO1 pulled to GND
 */
void bmi088_acc_init_i2c(bmi088_acc_device_t *device, uint8_t i2c, bool sdo1Grounded);

/**
 * @brief Validate BMI088 accelerometer device
 * 
 * @param device Accelerometer device
 * @return true if valid, false otherwise
 */
bool bmi088_acc_validate(const bmi088_acc_device_t *device);

/**
 * @brief Set BMI088 accelerometer configuration
 *
 * @param device Accelerometer device
 * @param odr ODR
 * @param osr OSR
 */
void bmi088_acc_set_conf(const bmi088_acc_device_t *device, bmi088_acc_odr_t odr, bmi088_acc_osr_t osr);

/**
 * @brief Set BMI088 accelerometer range
 *
 * @param device Accelerometer device
 * @param range Range
 */
void bmi088_acc_set_range(bmi088_acc_device_t *device, bmi088_acc_range_t range);

/**
 * @brief Read BMI088 accelerometer data
 *
 * @param device Accelerometer device
 * @param accel Acceleration in meters per second squared
 */
void bmi088_acc_read(const bmi088_acc_device_t *device, vec3_t *accel);

#ifdef __cplusplus
}
#endif

#endif