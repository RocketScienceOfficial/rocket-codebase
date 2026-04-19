#pragma once

#include "DriverBase.h"
#include <lib/drivers/gps/gps_driver.h>

class Driver_gps : public DriverBase<Driver_gps, PubSub::Topics::SensorsGPS>
{
public:
    Driver_gps() : DriverBase(PUBSUB_ID(sensors_gps_1)) {}

    void initialize();
    void readAndPublish(float dt);

private:
    gps_device_t m_Device;
};