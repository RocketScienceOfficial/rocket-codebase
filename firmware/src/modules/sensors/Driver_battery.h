#pragma once

#include "DriverBase.h"
#include <lib/drivers/adc/ads786x_driver.h>
#include <lib/battery/battery_interp_model.h>

#define BATTERY_READINGS_COUNT 10

class Driver_battery : public DriverBase<Driver_battery, PubSub::Topics::SensorsBattery>
{
public:
    Driver_battery() : DriverBase(PUBSUB_ID(sensors_battery), 10) {}

    void initialize();
    void readAndPublish(float dt);

private:
    ads786x_device_t m_Device;
    battery_config_t m_BatteryConfig;

    float m_BatteryReadings[BATTERY_READINGS_COUNT];
    size_t m_BatteryReadingsNextIndex = 0;
    float m_BatteryReadingsSum = 0;
    size_t m_BatteryReadingsCount = 0;

    void accumulateBatteryReading(float reading);
    void smoothenBatteryVoltage(float &delta);
};