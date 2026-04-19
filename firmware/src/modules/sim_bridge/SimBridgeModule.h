#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimBridgeModule
{
public:
    ~SimBridgeModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::SensorsIMU> m_IMU1DataPublisher{PUBSUB_ID(sensors_imu_1)};
    PubSub::Publisher<PubSub::Topics::SensorsMag> m_Mag1DataPublisher{PUBSUB_ID(sensors_mag_1)};
    PubSub::Publisher<PubSub::Topics::SensorsBaro> m_Baro1DataPublisher{PUBSUB_ID(sensors_baro_1)};
    PubSub::Publisher<PubSub::Topics::SensorsGPS> m_GPS1DataPublisher{PUBSUB_ID(sensors_gps_1)};
    PubSub::Publisher<PubSub::Topics::IgnAdcChannels> m_AdcIgnitersChannelsPublisher{PUBSUB_ID(ign_adc_channels)};
    PubSub::Publisher<PubSub::Topics::SensorsBattery> m_BatteryPublisher{PUBSUB_ID(sensors_battery)};

    PubSub::Subscriber<PubSub::Topics::EKFState> m_EKFStateSubscriber{PUBSUB_ID(ekf_state)};
    PubSub::Subscriber<PubSub::Topics::IgnFired> m_IGNSubscriber{PUBSUB_ID(ign_fired)};
    PubSub::Subscriber<PubSub::Topics::StateMachineState> m_StateMachineStateSubscriber{PUBSUB_ID(sm_state)};

    sitl_response_data m_responseData;

    network::TCPSocket m_PhysicsSocket;

    void receivePhysicsData();
    void sendPhysicsResponseData();
};