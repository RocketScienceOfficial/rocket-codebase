#pragma once

#pragma once

#include "DriverBase.h"
#include <lib/drivers/mag/mmc5983ma_driver.h>

class Driver_mmc5983ma : public DriverBase<Driver_mmc5983ma, PubSub::Topics::SensorsMag>
{
public:
    Driver_mmc5983ma() : DriverBase(PUBSUB_ID(sensors_mag_1), 100) {}

    void initialize();
    void readAndPublish(float dt);

private:
    mmc5983ma_device_t m_Device;
};