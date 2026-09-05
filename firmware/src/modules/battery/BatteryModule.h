#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include "battery_interp_model.h"

#define BATTERY_READINGS_COUNT 10

class BatteryModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_battery_topic> m_Publisher;
    PubSub::Subscriber<PubSub::Topics::sensors_battery_raw_topic> m_Subscriber;

    PubSub::Messages::SensorsBattery m_CurrentFrame;
    
    battery_config_t m_BatteryConfig;

    float m_BatteryReadings[BATTERY_READINGS_COUNT];
    size_t m_BatteryReadingsNextIndex = 0;
    float m_BatteryReadingsSum = 0;
    size_t m_BatteryReadingsCount = 0;

    void accumulateBatteryReading(float reading);
    void smoothenBatteryVoltage(float &delta);
};