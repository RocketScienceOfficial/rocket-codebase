#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <pubsub/Publisher.h>
#include <pubsub/RPCHandler.h>

#define IGN_COUNT 4

static_assert(PubSub::Helpers::IGN_CHANNELS_COUNT == IGN_COUNT, "IGN_CHANNELS_COUNT must be equal to IGN_COUNT");

class IgnitersModule
{
public:
    void init();
    void run();

private:
    PubSub::RPCHandler<PubSub::Topics::CommandIgnite> m_RPC_IGN{PUBSUB_RPC_ID(command_ignite)};
    PubSub::Subscriber<PubSub::Topics::IgnAdcChannels> m_ADCSubscriber{PUBSUB_ID(ign_adc_channels)};
    PubSub::Subscriber<PubSub::Topics::SensorsBattery> m_BatSubscriber{PUBSUB_ID(sensors_battery)};
    PubSub::Subscriber<PubSub::Topics::StateMachineState> m_SMSubscriber{PUBSUB_ID(sm_state)};
    PubSub::Subscriber<PubSub::Topics::StateMachineHeight> m_SMHeightSubscriber{PUBSUB_ID(sm_height)};
    PubSub::Subscriber<PubSub::Topics::EKFState> m_EKFSubscriber{PUBSUB_ID(ekf_state)};
    PubSub::Publisher<PubSub::Topics::IgnContinuity> m_IgnDetPublisher{PUBSUB_ID(ign_continuity)};
    PubSub::Publisher<PubSub::Topics::IgnFired> m_IgnFiredPublisher{PUBSUB_ID(ign_fired)};

    PubSub::Topics::IgnContinuity m_CurrentIgnContinuityPubData;
    PubSub::Topics::IgnFired m_CurrentIgnFiredPubData;

    struct IgniterPinData
    {
        uint8_t pin;
        bool fired;
        uint32_t fireTime;
        bool finished;
    };

    IgniterPinData m_Igniters[IGN_COUNT];

    bool m_ADCUpdate;
    bool m_ApogeeReached;
    IgniterPinData* m_CurrentTestingIgniter;

    void gatherData();
    void initIgniterPin(IgniterPinData &data, uint8_t pin);
    void ignTestFire();
    void ignFire(IgniterPinData &data);
    void ignUpdate(IgniterPinData &data);
    void ignFinish(IgniterPinData &data);
    void ignUpdateContinuity();
};