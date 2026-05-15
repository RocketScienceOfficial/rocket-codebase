#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Subscriber.h>
#include <pubsub/Publisher.h>

class AirbrakeNNModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::EKFState> m_EKFSubscriber{PUBSUB_ID(ekf_state)};
    PubSub::Publisher<PubSub::Topics::AirbrakeState> m_AirbrakePublisher{PUBSUB_ID(airbrake_state)};
};