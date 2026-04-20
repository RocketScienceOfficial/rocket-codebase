#include "mmc5983ma_driver.h"
#include <lib/debug/sys_assert.h>
#include <stdint.h>

static void _mmc5983ma_auto_set_reset(mmc5983ma_device_t *device)
{
    bus_utils_write_reg(&device->device, MMC5983MA_REG_CTRL0, MMC5983MA_CTRL0_AUTO_SR_EN);
}

static void _mmc5983ma_set(const mmc5983ma_device_t *device)
{
    bus_utils_write_reg(&device->device, MMC5983MA_REG_CTRL0, MMC5983MA_CTRL0_SET);
}

static void _mmc5983ma_reset(const mmc5983ma_device_t *device)
{
    bus_utils_write_reg(&device->device, MMC5983MA_REG_CTRL0, MMC5983MA_CTRL0_RESET);
}

static void _mmc5983ma_base_init(const mmc5983ma_device_t *device)
{
    _mmc5983ma_auto_set_reset((mmc5983ma_device_t *)device);
    _mmc5983ma_set(device);
    _mmc5983ma_reset(device);
}

void mmc5983ma_init_spi(mmc5983ma_device_t *device, uint8_t spi, uint8_t cs)
{
    SYS_ASSERT(device != NULL);
    
    bus_utils_init_spi_device(&device->device, spi, cs, MMC5983MA_READ_MASK, MMC5983MA_READ_MASK, MMC5983MA_WRITE_MASK);

    _mmc5983ma_base_init(device);
}

void mmc5983ma_init_i2c(mmc5983ma_device_t *device, uint8_t i2c)
{
    SYS_ASSERT(device != NULL);

    bus_utils_init_i2c_device(&device->device, i2c, MMC5983MA_I2C_ADDR, MMC5983MA_READ_MASK, MMC5983MA_READ_MASK, MMC5983MA_WRITE_MASK);

    _mmc5983ma_base_init(device);
}

bool mmc5983ma_validate(const mmc5983ma_device_t *device)
{
    SYS_ASSERT(device != NULL);

    return bus_utils_read_reg(&device->device, MMC5983MA_REG_PROD_ID1) == MMC5983MA_PRODUCT_ID;
}

void mmc5983ma_set_continuous_mode_odr(const mmc5983ma_device_t *device, mmc5983_odr_t odr)
{
    SYS_ASSERT(device != NULL);

    bus_utils_write_reg(&device->device, MMC5983MA_REG_CTRL1, 0x00);
    bus_utils_write_reg(&device->device, MMC5983MA_REG_CTRL2, 0x00);
    bus_utils_write_reg(&device->device, MMC5983MA_REG_CTRL2, (uint8_t)odr | MMC5983MA_CTRL2_CMM_EN);
}

void mmc5983ma_read(const mmc5983ma_device_t *device, vec3_t *mag)
{
    SYS_ASSERT(device != NULL);
    SYS_ASSERT(mag != NULL);

    uint8_t buffer[7];
    bus_utils_read_regs(&device->device, MMC5983MA_REG_XOUT0, buffer, 7);

    uint32_t rawX = (uint32_t)((buffer[0] << MMC5983MA_OUT0_SHIFT) | (buffer[1] << MMC5983MA_OUT1_SHIFT) | ((buffer[6] & MMC5983MA_XYZOUT2_X_MASK) >> MMC5983MA_XYZOUT2_X_SHIFT));
    uint32_t rawY = (uint32_t)((buffer[2] << MMC5983MA_OUT0_SHIFT) | (buffer[3] << MMC5983MA_OUT1_SHIFT) | ((buffer[6] & MMC5983MA_XYZOUT2_Y_MASK) >> MMC5983MA_XYZOUT2_Y_SHIFT));
    uint32_t rawZ = (uint32_t)((buffer[4] << MMC5983MA_OUT0_SHIFT) | (buffer[5] << MMC5983MA_OUT1_SHIFT) | ((buffer[6] & MMC5983MA_XYZOUT2_Z_MASK) >> MMC5983MA_XYZOUT2_Z_SHIFT));

    mag->x = ((float)rawX / (1 << 14) - 8) * 1000;
    mag->y = ((float)rawY / (1 << 14) - 8) * 1000;
    mag->z = ((float)rawZ / (1 << 14) - 8) * 1000;
}