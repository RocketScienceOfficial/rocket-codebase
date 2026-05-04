#include "Driver_gps.h"
#include <board_config.h>

void Driver_gps::initialize()
{
    gps_init_spi(&m_Device, CFG_SPI, CFG_PIN_CS_NEO);
}

void Driver_gps::readAndPublish(float dt)
{
    (void)dt;

    if (gps_read_spi(&m_Device))
    {
        if (m_Device.data.fix)
        {
            m_CurrentFrame.pos = m_Device.data.position;
            m_CurrentFrame.vel = m_Device.data.velocity_ned;
            m_CurrentFrame.std_horizontal = m_Device.data.std_horizontal;
            m_CurrentFrame.std_vertical = m_Device.data.std_vertical;
            m_CurrentFrame.std_speed = m_Device.data.std_speed;
            m_CurrentFrame.gpsFix = m_Device.data.fix;
            m_CurrentFrame.gpsIs3dFix = m_Device.data.is3dFix;
            m_CurrentFrame.gpsSatellitesCount = m_Device.data.numSV;

            m_Publisher.publish(m_CurrentFrame);
        }
    }
}