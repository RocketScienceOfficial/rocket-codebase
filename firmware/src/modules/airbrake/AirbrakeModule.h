#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <pubsub/Publisher.h>

class AirbrakeModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::ekf_state_topic> m_EKFSubscriber;
    PubSub::Publisher<PubSub::Topics::airbrake_state_topic> m_AirbrakePublisher;

    float m_LastPredictedApogee;
};