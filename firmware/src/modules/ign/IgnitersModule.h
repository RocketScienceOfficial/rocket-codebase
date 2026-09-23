#pragma once

#include "IgnConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <pubsub/Publisher.h>
#include <pubsub/RPCHandler.h>
#include <hal/gpio_driver.h>


static_assert(PubSub::Helpers::IGN_CHANNELS_COUNT == IGN_COUNT, "IGN_CHANNELS_COUNT must be equal to IGN_COUNT");

class IgnitersModule
{
public:
    IgnitersModule(hal_gpio_pin_t ign1, hal_gpio_pin_t ign2, hal_gpio_pin_t ign3, hal_gpio_pin_t ign4);

    void init();
    void run();

private:
    PubSub::RPCHandler<PUBSUB_RPC_ID(command_ignite)> m_RPC_IGN;
    PubSub::Subscriber<PubSub::Topics::ign_adc_channels_topic> m_ADCSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_battery_topic> m_BatSubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_SMSubscriber;
    PubSub::Subscriber<PubSub::Topics::ekf_state_topic> m_EKFSubscriber;
    PubSub::Publisher<PubSub::Topics::ign_continuity_topic> m_IgnDetPublisher;
    PubSub::Publisher<PubSub::Topics::ign_fired_topic> m_IgnFiredPublisher;

    PubSub::Messages::IgnContinuity m_CurrentIgnContinuityPubData{};
    PubSub::Messages::IgnFired m_CurrentIgnFiredPubData{};

    struct IgniterPinData
    {
        hal_gpio_pin_t pin;
        bool fired;
        uint32_t fireTime;
        bool finished;
    };

    IgniterPinData m_Igniters[IGN_COUNT];

    uint32_t m_LastContinuityUpdateTime = 0;
    bool m_StatusUpdate = false;
    bool m_ApogeeReached = false;
    IgniterPinData *m_CurrentTestingIgniter = nullptr;

    void gatherData();
    void updateFiredStatus();
    void updateContinuity();
    void initIgniterPin(IgniterPinData &data);
    void testIgniter();
    void fireIgniter(IgniterPinData &data);
    void updateIgniter(IgniterPinData &data);
    void finishFire(IgniterPinData &data);
};