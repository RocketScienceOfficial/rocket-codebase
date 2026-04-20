#include "Driver_ms5611.h"
#include <board_config.h>
#include <lib/maths/math_utils.h>
#include <lib/geo/geo_utils.h>

#define EXP_FILTER_BARO_COEFF 0.05f

void Driver_ms5611::initialize()
{
    ms56xx_init_spi(&m_Device, CFG_SPI, CFG_PIN_CS_MS56, true);
    ms56xx_set_osr(&m_Device, MS56XX_OSR_512, MS56XX_OSR_512);
}

void Driver_ms5611::readAndPublish(float dt)
{
    (void)dt;

    static int lastPress = 0;

    if (ms56xx_read_non_blocking(&m_Device, &m_CurrentFrame.press, &m_CurrentFrame.temp))
    {
        m_CurrentFrame.press = exp_smoothing(m_CurrentFrame.press, lastPress, EXP_FILTER_BARO_COEFF);
        m_CurrentFrame.baroHeight = height_from_baro_formula(m_CurrentFrame.press);

        lastPress = m_CurrentFrame.press;

        m_Publisher.publish(m_CurrentFrame);
    }
}