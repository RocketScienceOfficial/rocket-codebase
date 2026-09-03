#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include "ws2812b_driver.h"

#define DIODES_COUNT 7

class LEDModule
{
public:
    LEDModule(const hal_waveform_channel_t ledChannel, const hal_gpio_pin_t ledPin)
        : m_LEDChannel(ledChannel), m_LEDPin(ledPin) {}

    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::ign_continuity_topic> m_ContinuitySubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_battery_topic> m_BatSubscriber;
    PubSub::Subscriber<PubSub::Topics::database_ready_topic> m_ReadySubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_StateSubscriber;

    const hal_waveform_channel_t m_LEDChannel;
    const hal_gpio_pin_t m_LEDPin;

    ws2812b_color_t m_DiodesColors[DIODES_COUNT];
    bool m_Updated = false;

    void setIgniterValue(uint8_t igniterNumber, bool fuseWorking, bool ignPresent);
    void setArmState(bool armed);
    void setBatteryPercentage(uint8_t percent);
    void setReadyState(bool ready);
    void update();
};