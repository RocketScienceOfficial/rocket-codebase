#include "mmc5983ma_Module.h"

void mmc5983ma_Module::init()
{
    mmc5983ma_init_spi(&m_Device, m_SPI, m_CS);
    mmc5983ma_set_continuous_mode_odr(&m_Device, MMC5983MA_ODR_100HZ);
}

void mmc5983ma_Module::run()
{
    mmc5983ma_read(&m_Device, &m_CurrentFrame.mag);

    // FRD conversion
    m_CurrentFrame.mag.y *= -1;

    m_Publisher.publish(m_CurrentFrame);
}