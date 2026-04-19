#pragma once

#include "DriverBase.h"
#include <lib/drivers/baro/ms56xx_driver.h>

class Driver_ms5611 : public DriverBase<Driver_ms5611, PubSub::Topics::SensorsBaro>
{
public:
    Driver_ms5611() : DriverBase(PUBSUB_ID(sensors_baro_1)) {}

    void initialize();
    void readAndPublish(float dt);

private:
    ms56xx_device_t m_Device;
};