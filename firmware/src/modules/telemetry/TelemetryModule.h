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
    PubSub::Publisher<PubSub::Topics::telemetry_tx_topic> m_TelemetryTXPublisher;
    PubSub::Subscriber<PubSub::Topics::telemetry_rx_topic> m_RadioResponseSubscriber;
    PubSub::Subscriber<PubSub::Topics::radio_ack_topic> m_RadioAckSubscriber;
    PubSub::Subscriber<PubSub::Topics::commander_state_topic> m_CommanderSubscriber;
    PubSub::Subscriber<PubSub::Topics::ekf_state_topic> m_EKFStateSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_battery_topic> m_BatterySubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_gps_1_topic> m_GPSSubscriber;
    PubSub::Subscriber<PubSub::Topics::sm_state_topic> m_StateMachineStateSubscriber;
    PubSub::Subscriber<PubSub::Topics::voltage_state_topic> m_VoltageStateSubscriber;
    PubSub::Subscriber<PubSub::Topics::ign_continuity_topic> m_IgnitionContinuitySubscriber;

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