#include "bmi088_acc_driver.h"
#include "time_utils.h"
#include <lib/debug/sys_assert.h>
#include <lib/geo/physical_constants.h>
#include <lib/maths/math_constants.h>
#include <math.h>
#include <string.h>

static uint8_t _bmi088_acc_read_reg(const bmi088_acc_device_t *device, uint8_t address)
{
    uint8_t data[2];

    bus_utils_read_regs(&device->device, address, data, 2);

    return data[1];
}

static void _bmi088_acc_read_regs(const bmi088_acc_device_t *device, uint8_t address, uint8_t *buffer, size_t count)
{
    uint8_t tmp_buffer[16];

    bus_utils_read_regs(&device->device, address, tmp_buffer, count + 1);

    memcpy(buffer, tmp_buffer + 1, count);
}

static void _bmi088_acc_write_reg(const bmi088_acc_device_t *device, uint8_t address, uint8_t data)
{
    bus_utils_write_reg(&device->device, address, data);
}

static void _bmi088_acc_set_power(const bmi088_acc_device_t *device, bool on)
{
    uint8_t data = on ? BMI088_ACC_PWR_CTRL_ON : BMI088_ACC_PWR_CTRL_OFF;

    _bmi088_acc_write_reg(device, BMI088_ACC_REG_PWR_CTRL, data);

    time_utils_delay_us_osal(BMI088_ACC_PWR_CTRL_TIMEOUT_US);
}

static void _bmi088_acc_set_mode(const bmi088_acc_device_t *device, bool active)
{
    uint8_t data = active ? BMI088_ACC_PWR_CONF_ACTIVE : BMI088_ACC_PWR_CONF_SUSPEND;

    _bmi088_acc_write_reg(device, BMI088_ACC_REG_PWR_CONF, data);

    time_utils_delay_us_osal(BMI088_ACC_PWR_CONF_TIMEOUT_US);
}

static void _bmi088_acc_init_base(bmi088_acc_device_t *device)
{
    _bmi088_acc_set_power(device, true);
    _bmi088_acc_set_mode(device, true);
}

void bmi088_acc_init_spi(bmi088_acc_device_t *device, uint8_t spi, uint8_t cs)
{
    SYS_ASSERT(device != NULL);

    device->rangeFactor = 0.0f;

    bus_utils_init_spi_device(&device->device, spi, cs, BMI088_ACC_READ_MASK, BMI088_ACC_READ_MASK, BMI088_ACC_WRITE_MASK);

    _bmi088_acc_init_base(device);
}

void bmi088_acc_init_i2c(bmi088_acc_device_t *device, uint8_t i2c, bool sdo1Grounded)
{
    SYS_ASSERT(device != NULL);

    device->rangeFactor = 0.0f;

    uint8_t address = sdo1Grounded ? BMI088_ACC_GND_I2C_ADDRESS : BMI088_ACC_VDD_I2C_ADDRESS;

    bus_utils_init_i2c_device(&device->device, i2c, address, BMI088_ACC_READ_MASK, BMI088_ACC_READ_MASK, BMI088_ACC_WRITE_MASK);

    _bmi088_acc_init_base(device);
}

bool bmi088_acc_validate(const bmi088_acc_device_t *device)
{
    SYS_ASSERT(device != NULL);

    return _bmi088_acc_read_reg(device, BMI088_ACC_REG_CHIP_ID) == BMI088_ACC_CHIP_ID;
}

void bmi088_acc_set_conf(const bmi088_acc_device_t *device, bmi088_acc_odr_t odr, bmi088_acc_osr_t osr)
{
    SYS_ASSERT(device != NULL);

    _bmi088_acc_write_reg(device, BMI088_ACC_REG_CONF, (uint8_t)odr | ((uint8_t)osr << BMI088_ACC_CONF_OSR_BITSHIFT));

    time_utils_delay_us_osal(BMI088_ACC_CONF_TIMEOUT_US);
}

void bmi088_acc_set_range(bmi088_acc_device_t *device, bmi088_acc_range_t range)
{
    SYS_ASSERT(device != NULL);

    device->rangeFactor = powf(2, range + 1) * 1.5f * EARTH_GRAVITY / (1 << 15);

    _bmi088_acc_write_reg(device, BMI088_ACC_REG_RANGE, (uint8_t)range);
}

void bmi088_acc_read(const bmi088_acc_device_t *device, vec3_t *accel)
{
    SYS_ASSERT(device != NULL);
    SYS_ASSERT(accel != NULL);

    uint8_t buff[6];

    _bmi088_acc_read_regs(device, BMI088_ACC_REG_DATA, buff, 6);

    int16_t accelX = (int16_t)((buff[1] << 8) | buff[0]);
    int16_t accelY = (int16_t)((buff[3] << 8) | buff[2]);
    int16_t accelZ = (int16_t)((buff[5] << 8) | buff[4]);

    accel->x = accelX * device->rangeFactor;
    accel->y = accelY * device->rangeFactor;
    accel->z = accelZ * device->rangeFactor;
}