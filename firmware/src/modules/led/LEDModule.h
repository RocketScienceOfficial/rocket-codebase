#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <hal/ws2812b_driver.h>

#define DIODES_COUNT 7
#define BRIGHTNESS 0.05f

class LEDModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::IgnContinuity> m_ContinuitySubscriber{PUBSUB_ID(ign_continuity)};
    PubSub::Subscriber<PubSub::Topics::SensorsBattery> m_BatSubscriber{PUBSUB_ID(sensors_battery)};
    PubSub::Subscriber<PubSub::Topics::DatabaseReady> m_ReadySubscriber{PUBSUB_ID(database_ready)};
    PubSub::Subscriber<PubSub::Topics::StateMachineState> m_StateSubscriber{PUBSUB_ID(sm_state)};

    hal_ws2812b_color_t m_DiodesColors[DIODES_COUNT];
    bool m_Updated;

    void setIgniterValue(uint8_t igniterNumber, bool fuseWorking, bool ignPresent);
    void setArmState(bool armed);
    void setBatteryPercentage(uint8_t percent);
    void setReadyState(bool ready);
    void update();
};