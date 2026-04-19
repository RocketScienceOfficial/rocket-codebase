#pragma once

#include "Driver_adc_ign.h"
#include "Driver_battery.h"
#include "Driver_bmi088.h"
#include "Driver_gps.h"
#include "Driver_mmc5983ma.h"
#include "Driver_ms5611.h"

class SensorsModule
{
public:
    void init();
    void run();

private:
    Driver_adc_ign m_Driver_adc_ign;
    Driver_battery m_Driver_battery;
    Driver_bmi088 m_Driver_bmi088;
    Driver_gps m_Driver_gps;
    Driver_mmc5983ma m_Driver_mmc5983ma;
    Driver_ms5611 m_Driver_ms5611;
};