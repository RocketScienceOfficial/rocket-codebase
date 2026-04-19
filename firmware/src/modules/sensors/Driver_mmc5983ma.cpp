#include "Driver_mmc5983ma.h"
#include <board_config.h>

void Driver_mmc5983ma::initialize()
{
    mmc5983ma_init_spi(&m_Device, OBC_SPI, PIN_CS_MMC);
    mmc5983ma_set_continuous_mode_odr(&m_Device, MMC5983MA_ODR_1000HZ);
}

void Driver_mmc5983ma::readAndPublish(float dt)
{
    (void)dt;

    mmc5983ma_read(&m_Device, &m_CurrentFrame.mag);

    m_CurrentFrame.mag.x += +98;
    m_CurrentFrame.mag.y += +373;
    m_CurrentFrame.mag.z += -194;

    m_CurrentFrame.mag.x *= -1;
    m_CurrentFrame.mag.y *= -1;
    m_CurrentFrame.mag.z *= -1;

    m_Publisher.publish(m_CurrentFrame);
}