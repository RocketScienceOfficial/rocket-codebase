#include "bmi088_gyro_driver.h"
#include "time_utils.h"
#include <lib/debug/sys_assert.h>
#include <lib/geo/physical_constants.h>
#include <lib/maths/math_constants.h>
#include <math.h>

void bmi088_gyro_init_spi(bmi088_gyro_device_t *device, uint8_t spi, uint8_t cs)
{
    SYS_ASSERT(device != NULL);

    device->rangeFactor = 0.0f;

    bus_utils_init_spi_device(&device->device, spi, cs, BMI088_GYRO_READ_MASK, BMI088_GYRO_READ_MASK, BMI088_GYRO_WRITE_MASK);
}

void bmi088_gyro_init_i2c(bmi088_gyro_device_t *device, uint8_t i2c, bool sdo1Grounded)
{
    SYS_ASSERT(device != NULL);

    device->rangeFactor = 0.0f;

    uint8_t address = sdo1Grounded ? BMI088_GYRO_GND_I2C_ADDRESS : BMI088_GYRO_VDD_I2C_ADDRESS;

    bus_utils_init_i2c_device(&device->device, i2c, address, BMI088_GYRO_READ_MASK, BMI088_GYRO_READ_MASK, BMI088_GYRO_WRITE_MASK);
}

bool bmi088_gyro_validate(const bmi088_gyro_device_t *device)
{
    SYS_ASSERT(device != NULL);

    return bus_utils_read_reg(&device->device, BMI088_GYRO_CHIP_ID) == BMI088_GYRO_CHIP_ID;
}

void bmi088_gyro_set_bandwidth(const bmi088_gyro_device_t *device, bmi088_gyro_bandwidth_t bw)
{
    SYS_ASSERT(device != NULL);

    bus_utils_write_reg(&device->device, BMI088_GYRO_REG_BANDWIDTH, (uint8_t)bw);

    time_utils_delay_us_osal(BMI088_GYRO_BANDWIDTH_TIMEOUT_US);
}

void bmi088_gyro_set_range(bmi088_gyro_device_t *device, bmi088_gyro_range_t range)
{
    SYS_ASSERT(device != NULL);

    device->rangeFactor = DEG_2_RAD(2000.0f / powf(2, range)) / (1 << 15);

    bus_utils_write_reg(&device->device, BMI088_GYRO_REG_RANGE, (uint8_t)range);

    time_utils_delay_us_osal(BMI088_GYRO_RANGE_TIMEOUT_US);
}

void bmi088_gyro_read(const bmi088_gyro_device_t *device, vec3_t *gyro)
{
    SYS_ASSERT(device != NULL);
    SYS_ASSERT(gyro != NULL);

    uint8_t buff[6];

    bus_utils_read_regs(&device->device, BMI088_GYRO_REG_RATE, buff, 6);

    int16_t gyroX = (int16_t)((buff[1] << 8) | buff[0]);
    int16_t gyroY = (int16_t)((buff[3] << 8) | buff[2]);
    int16_t gyroZ = (int16_t)((buff[5] << 8) | buff[4]);

    gyro->x = gyroX * device->rangeFactor;
    gyro->y = gyroY * device->rangeFactor;
    gyro->z = gyroZ * device->rangeFactor;
}