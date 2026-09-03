#include "Driver_mmc5983ma.h"
#include <board_config.h>

void Driver_mmc5983ma::initialize()
{
    mmc5983ma_init_spi(&m_Device, CFG_SPI, CFG_PIN_CS_MMC);
    mmc5983ma_set_continuous_mode_odr(&m_Device, MMC5983MA_ODR_100HZ);
}

void Driver_mmc5983ma::readAndPublish(float dt)
{
    (void)dt;

    mmc5983ma_read(&m_Device, &m_CurrentFrame.mag);

    // FRD conversion
    m_CurrentFrame.mag.y *= -1;

    m_Publisher.publish(m_CurrentFrame);
}