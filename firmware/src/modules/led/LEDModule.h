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
    PubSub::Subscriber<PubSub::Topics::ign_continuity_topic> m_ContinuitySubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_battery_topic> m_BatSubscriber;
    PubSub::Subscriber<PubSub::Topics::database_ready_topic> m_ReadySubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_StateSubscriber;

    hal_ws2812b_color_t m_DiodesColors[DIODES_COUNT];
    bool m_Updated;

    void setIgniterValue(uint8_t igniterNumber, bool fuseWorking, bool ignPresent);
    void setArmState(bool armed);
    void setBatteryPercentage(uint8_t percent);
    void setReadyState(bool ready);
    void update();
};