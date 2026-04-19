#pragma once

#include "DriverBase.h"
#include <lib/drivers/imu/bmi088_acc_driver.h>
#include <lib/drivers/imu/bmi088_gyro_driver.h>

class Driver_bmi088 : public DriverBase<Driver_bmi088, PubSub::Topics::SensorsIMU>
{
public:
    Driver_bmi088() : DriverBase(PUBSUB_ID(sensors_imu_1), 500) {}

    void initialize();
    void readAndPublish(float dt);

private:
    bmi088_acc_device_t m_AccDevice;
    bmi088_gyro_device_t m_GyroDevice;
};