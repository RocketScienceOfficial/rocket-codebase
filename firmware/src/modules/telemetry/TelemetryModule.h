#pragma once

#include <datalink.h>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstdint>

class TelemetryModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::TelemetryDataOBC> m_TelemetryTXPublisher{PUBSUB_ID(telemetry_tx)};
    PubSub::Subscriber<PubSub::Topics::TelemetryResponse> m_RadioResponseSubscriber{PUBSUB_ID(telemetry_rx)};
    PubSub::Subscriber<PubSub::Topics::RadioAck> m_RadioAckSubscriber{PUBSUB_ID(radio_ack)};
    PubSub::Subscriber<PubSub::Topics::CommanderState> m_CommanderSubscriber{PUBSUB_ID(commander_state)};
    PubSub::Subscriber<PubSub::Topics::EKFState> m_EKFStateSubscriber{PUBSUB_ID(ekf_state)};
    PubSub::Subscriber<PubSub::Topics::SensorsBattery> m_BatterySubscriber{PUBSUB_ID(sensors_battery)};
    PubSub::Subscriber<PubSub::Topics::SensorsGPS> m_GPSSubscriber{PUBSUB_ID(sensors_gps_1)};
    PubSub::Subscriber<PubSub::Topics::StateMachineState> m_StateMachineStateSubscriber{PUBSUB_ID(sm_state)};
    PubSub::Subscriber<PubSub::Topics::VoltageState> m_VoltageStateSubscriber{PUBSUB_ID(voltage_state)};
    PubSub::Subscriber<PubSub::Topics::IgnContinuity> m_IgnitionContinuitySubscriber{PUBSUB_ID(ign_continuity)};

    uint32_t m_RadioTXDoneRecoveryTimeOffset;
    uint32_t m_RadioResponseRecoveryTimeOffset;
    uint32_t m_PacketTimer;
    bool m_WaitingToSendPacket;
    int m_PacketCounterForResponse;
    bool m_WaitingForResponse;

    void handleAck();
    void handleResponse();
    void scheduleNextPacket();

    uint8_t packetGetCommanderState();
    uint16_t packetGetAltitude();
    uint8_t packetGetGPSData();
    uint8_t packetGetStateFlags();
    bool packetCheckIgnCont(uint8_t ign);
};