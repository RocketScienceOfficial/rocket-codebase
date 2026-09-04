#include "ads786x_Module.h"

void ads786x_Module::init()
{
    ads786x_init(&m_Device, m_SPI, m_CS, ADS786X_TYPE_6, m_VRef);
}

void ads786x_Module::run()
{
    m_CurrentFrame.rawVoltage = ads786x_read(&m_Device) * m_CalibScale + m_CalibOffset;
    m_Publisher.publish(m_CurrentFrame);
}