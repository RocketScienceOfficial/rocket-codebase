#pragma once

#include "DriverBase.h"
#include <lib/drivers/gps/gps_driver.h>

class Driver_gps : public DriverBase<Driver_gps, PubSub::Topics::sensors_gps_1_topic>
{
public:
    Driver_gps() {}

    void initialize();
    void readAndPublish(float dt);

private:
    gps_device_t m_Device;
};