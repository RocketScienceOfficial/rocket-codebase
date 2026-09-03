#include "ms56xx_driver.h"
#include "spi_utils.h"
#include "time_utils.h"
#include <lib/debug/sys_assert.h>
#include <hal/spi_driver.h>
#include <hal/time_driver.h>

static void ms56xx_reset(const ms56xx_device_t *device)
{
    uint8_t data = MS56XX_CMD_RESET;

    spi_utils_cs_select(device->cs);
    hal_spi_transfer(device->spi, &data, NULL, 1);
    spi_utils_cs_deselect(device->cs);

    time_utils_delay_us_osal(MS56XX_TIMEOUT_RESET_US);
}

/**
 * REF: https://github.com/PX4/PX4-Autopilot/blob/8f5f564c05b5a0e12989d6e56642cc0b453ec45d/src/drivers/barometer/ms56xx/ms56xx.cpp#L352
 */
static bool ms56xx_validate_crc(uint16_t *n_prom)
{
    int16_t cnt;
    uint16_t n_rem;
    uint16_t crc_read;
    uint8_t n_bit;

    n_rem = 0x00;

    /* save the read crc */
    crc_read = n_prom[7];

    /* remove CRC byte */
    n_prom[7] = (0xFF00 & (n_prom[7]));

    for (cnt = 0; cnt < 16; cnt++)
    {
        /* uneven bytes */
        if (cnt & 1)
        {
            n_rem ^= (uint8_t)((n_prom[cnt >> 1]) & 0x00FF);
        }
        else
        {
            n_rem ^= (uint8_t)(n_prom[cnt >> 1] >> 8);
        }

        for (n_bit = 8; n_bit > 0; n_bit--)
        {
            if (n_rem & 0x8000)
            {
                n_rem = (n_rem << 1) ^ 0x3000;
            }
            else
            {
                n_rem = (n_rem << 1);
            }
        }
    }

    /* final 4 bit remainder is CRC value */
    n_rem = (0x000F & (n_rem >> 12));
    n_prom[7] = crc_read;

    /* return true if CRCs match */
    return (0x000F & crc_read) == (n_rem ^ 0x00);
}

static bool ms56xx_read_coefficents(ms56xx_device_t *device)
{
    ms56xx_prom_data_t coeffs = {0};

    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t tx[3] = {MS56XX_CMD_PROM_READ_BASE + (i << 1), 0x00, 0x00};
        uint8_t rx[3];

        spi_utils_cs_select(device->cs);
        hal_spi_transfer(device->spi, tx, rx, sizeof(tx));
        spi_utils_cs_deselect(device->cs);

        ((uint16_t *)&coeffs)[i] = rx[1] << 8 | rx[2];
    }

    if (!ms56xx_validate_crc((uint16_t *)&coeffs))
    {
        return false;
    }

    device->coeffs = coeffs;

    return true;
}

void ms56xx_init_spi(ms56xx_device_t *device, uint8_t spi, uint8_t cs, bool version_5611)
{
    SYS_ASSERT(device != NULL);

    device->spi = spi;
    device->cs = cs;
    device->version_5611 = version_5611;
    device->d1 = 0;

    spi_utils_cs_init(cs);

    ms56xx_reset(device);

    device->coeffs_valid = ms56xx_read_coefficents(device);
}

void ms56xx_set_osr(ms56xx_device_t *device, ms56xx_osr_t press, ms56xx_osr_t temp)
{
    SYS_ASSERT(device != NULL);

    device->pressOSR = press;
    device->tempOSR = temp;
}

bool ms56xx_validate(const ms56xx_device_t *device)
{
    SYS_ASSERT(device != NULL);

    return device->coeffs_valid;
}

static uint32_t ms56xx_read_raw_value(const ms56xx_device_t *device)
{
    uint8_t tx[4] = {MS56XX_CMD_ADC_READ, 0x00, 0x00, 0x00};
    uint8_t rx[4];

    spi_utils_cs_select(device->cs);
    hal_spi_transfer(device->spi, tx, rx, sizeof(tx));
    spi_utils_cs_deselect(device->cs);

    uint32_t d = (uint32_t)((uint32_t)rx[1] << 16 | (uint32_t)rx[2] << 8 | (uint32_t)rx[3]);

    return d;
}

static uint32_t ms56xx_get_timeout(ms56xx_osr_t osr)
{
    switch (osr)
    {
    case MS56XX_OSR_256:
        return MS56XX_TIMEOUT_OSR_256_US;
    case MS56XX_OSR_512:
        return MS56XX_TIMEOUT_OSR_512_US;
    case MS56XX_OSR_1024:
        return MS56XX_TIMEOUT_OSR_1024_US;
    case MS56XX_OSR_2048:
        return MS56XX_TIMEOUT_OSR_2048_US;
    case MS56XX_OSR_4096:
        return MS56XX_TIMEOUT_OSR_4096_US;
    default:
        return 0;
    }
}

static void ms56xx_req_value(ms56xx_device_t *device, bool pressure)
{
    uint8_t data = pressure ? (uint8_t)device->pressOSR : ((uint8_t)device->tempOSR + MS56XX_CMD_CONVERT_D2_OSR_256 - MS56XX_CMD_CONVERT_D1_OSR_256);

    spi_utils_cs_select(device->cs);
    hal_spi_transfer(device->spi, &data, NULL, 1);
    spi_utils_cs_deselect(device->cs);

    ms56xx_osr_t osr = pressure ? device->pressOSR : device->tempOSR;
    uint32_t timeout = ms56xx_get_timeout(osr);

    SYS_ASSERT(timeout > 0);

    device->nextTime = hal_time_get_us_since_boot() + timeout;
}

static void ms5611_convert_raw_values(const ms56xx_prom_data_t *coeffs, uint32_t d1, uint32_t d2, int *pressure, float *temperature)
{
    int32_t dT = (int32_t)d2 - ((int32_t)coeffs->c5 << 8);
    int32_t temp = 2000 + (((int64_t)dT * coeffs->c6) >> 23);
    int64_t off = ((int64_t)coeffs->c2 << 16) + (((int64_t)coeffs->c4 * dT) >> 7);
    int64_t sens = ((int64_t)coeffs->c1 << 15) + (((int64_t)coeffs->c3 * dT) >> 8);

    if (temp < 2000)
    {
        int32_t t2 = ((int64_t)dT * (int64_t)dT) >> 31;
        int32_t temp_2000 = temp - 2000;
        int64_t off2 = (5 * (int64_t)temp_2000 * (int64_t)temp_2000) >> 1;
        int64_t sens2 = (5 * (int64_t)temp_2000 * (int64_t)temp_2000) >> 2;

        if (temp < -1500)
        {
            int32_t temp_1500 = temp + 1500;

            off2 += 7 * (int64_t)temp_1500 * (int64_t)temp_1500;
            sens2 += 11 * (int64_t)temp_1500 * (int64_t)temp_1500 / 2;
        }

        temp -= t2;
        off -= off2;
        sens -= sens2;
    }

    int32_t p = ((((int64_t)d1 * sens) >> 21) - off) >> 15;

    *temperature = (float)temp / 100.0f;
    *pressure = p;
}

static void ms5607_convert_raw_values(const ms56xx_prom_data_t *coeffs, uint32_t d1, uint32_t d2, int *pressure, float *temperature)
{
    int32_t dT = (int32_t)d2 - ((int32_t)coeffs->c5 << 8);
    int32_t temp = 2000 + (((int64_t)dT * coeffs->c6) >> 23);
    int64_t off = ((int64_t)coeffs->c2 << 17) + (((int64_t)coeffs->c4 * dT) >> 6);
    int64_t sens = ((int64_t)coeffs->c1 << 16) + (((int64_t)coeffs->c3 * dT) >> 7);

    if (temp < 2000)
    {
        int32_t t2 = ((int64_t)dT * (int64_t)dT) >> 31;
        int32_t temp_2000 = temp - 2000;
        int64_t off2 = (61 * (int64_t)temp_2000 * (int64_t)temp_2000) >> 4;
        int64_t sens2 = 2 * (int64_t)temp_2000 * (int64_t)temp_2000;

        if (temp < -1500)
        {
            int32_t temp_1500 = temp + 1500;

            off2 += 15 * (int64_t)temp_1500 * (int64_t)temp_1500;
            sens2 += 8 * (int64_t)temp_1500 * (int64_t)temp_1500;
        }

        temp -= t2;
        off -= off2;
        sens -= sens2;
    }

    int32_t p = ((((int64_t)d1 * sens) >> 21) - off) >> 15;

    *temperature = (float)temp / 100.0f;
    *pressure = p;
}

bool ms56xx_read_non_blocking(ms56xx_device_t *device, int *pressure, float *temperature)
{
    SYS_ASSERT(device != NULL);
    SYS_ASSERT(pressure != NULL);
    SYS_ASSERT(temperature != NULL);

    bool result = false;

    if (hal_time_get_us_since_boot() >= device->nextTime)
    {
        if (device->d1 == 0)
        {
            device->d1 = ms56xx_read_raw_value(device);

            ms56xx_req_value(device, false);
        }
        else
        {
            uint32_t d2 = ms56xx_read_raw_value(device);

            if (device->version_5611)
            {
                ms5611_convert_raw_values(&device->coeffs, device->d1, d2, pressure, temperature);
            }
            else
            {
                ms5607_convert_raw_values(&device->coeffs, device->d1, d2, pressure, temperature);
            }

            device->nextTime = 0;
            device->d1 = 0;

            result = true;
        }
    }

    if (device->nextTime == 0)
    {
        ms56xx_req_value(device, true);
    }

    return result;
}