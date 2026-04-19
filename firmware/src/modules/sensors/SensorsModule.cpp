#include "SensorsModule.h"

void SensorsModule::init()
{
    m_Driver_adc_ign.init();
    m_Driver_battery.init();
    m_Driver_bmi088.init();
    m_Driver_gps.init();
    m_Driver_mmc5983ma.init();
    m_Driver_ms5611.init();
}

void SensorsModule::run()
{
    m_Driver_adc_ign.update();
    m_Driver_battery.update();
    m_Driver_bmi088.update();
    m_Driver_gps.update();
    m_Driver_mmc5983ma.update();
    m_Driver_ms5611.update();
}