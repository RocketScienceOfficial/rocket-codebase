#pragma once

#include "DriverBase.h"
#include <lib/drivers/baro/ms56xx_driver.h>

class Driver_ms5611 : public DriverBase<Driver_ms5611, PubSub::Topics::sensors_baro_1_topic>
{
public:
    Driver_ms5611() {}

    void initialize();
    void readAndPublish(float dt);

private:
    ms56xx_device_t m_Device;
};