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

    float m_LastPredictedApogee;

    float m_Buffer1[64];
    float m_Buffer2[64];

    float runNeuralNet(float h0, float v0);
};