#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimBridgeModule
{
public:
    SimBridgeModule(uint16_t port) : m_Port(port) {}
    ~SimBridgeModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_imu_1_topic> m_IMU1DataPublisher;
    PubSub::Publisher<PubSub::Topics::sensors_mag_1_topic> m_Mag1DataPublisher;
    PubSub::Publisher<PubSub::Topics::sensors_baro_1_topic> m_Baro1DataPublisher;
    PubSub::Publisher<PubSub::Topics::sensors_gps_1_topic> m_GPS1DataPublisher;
    PubSub::Publisher<PubSub::Topics::ign_adc_channels_topic> m_AdcIgnitersChannelsPublisher;
    PubSub::Publisher<PubSub::Topics::sensors_battery_topic> m_BatteryPublisher;

    PubSub::Subscriber<PubSub::Topics::ekf_state_topic> m_EKFStateSubscriber;
    PubSub::Subscriber<PubSub::Topics::ign_fired_topic> m_IGNSubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_StateMachineStateSubscriber;
    PubSub::Subscriber<PubSub::Topics::airbrake_state_topic> m_AirbrakeStateSubscriber;

    sitl_response_data m_ResponseData{};

    const uint16_t m_Port;
    network::TCPSocket m_PhysicsSocket;

    void receivePhysicsData();
    void sendPhysicsResponseData();
};