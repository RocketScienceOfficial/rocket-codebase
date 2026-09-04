#include "ms56xx_Module.h"
#include <lib/geo/geo_utils.h>

void ms56xx_Module::init()
{
    ms56xx_init_spi(&m_Device, m_SPI, m_CS, m_Version5611);
    ms56xx_set_osr(&m_Device, MS56XX_OSR_4096, MS56XX_OSR_4096);
}

void ms56xx_Module::run()
{
    if (ms56xx_read_non_blocking(&m_Device, &m_CurrentFrame.press, &m_CurrentFrame.temp))
    {
        m_CurrentFrame.baroHeight = height_from_baro_formula(m_CurrentFrame.press);

        m_Publisher.publish(m_CurrentFrame);
    }
}