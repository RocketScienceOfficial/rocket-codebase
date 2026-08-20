#pragma once

#include "GCSCommandHandler.h"
#include <datalink.h>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstdint>

class CommanderGCSModule
{
public:
    CommanderGCSModule() : m_CommandHandler(m_SerialPublisher, m_CommandTimeoutPublisher) {}

    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::serial_rx_topic> m_SerialSubscriber;
    PubSub::Publisher<PubSub::Topics::serial_tx_topic> m_SerialPublisher;
    PubSub::Subscriber<PubSub::Topics::lora_rx_topic> m_RadioSubscriber;
    PubSub::Publisher<PubSub::Topics::lora_tx_topic> m_RadioPublisher;

    PubSub::Subscriber<PubSub::Topics::sensors_simplified_gps_1_topic> m_GPSSubscriber;

    PubSub::Publisher<PubSub::Topics::gcs_commander_timeout_topic> m_CommandTimeoutPublisher;
    PubSub::Publisher<PubSub::Topics::gcs_radio_state_topic> m_RadioStatePublisher;

    GCSCommandHandler<PubSub::Topics::serial_tx_topic, PubSub::Topics::gcs_commander_timeout_topic> m_CommandHandler;

    uint8_t m_RXSequence;
    uint8_t m_TXSequence;
    uint32_t m_PacketsLost;
    uint32_t m_RadioRX;
    uint32_t m_RadioTmpRX;
    uint32_t m_RadioTX;
    uint32_t m_ResponseStartTime;
    uint32_t m_RadioTmpRXStartTime;

    void processSerialMessage(const datalink_message_t &msg);
    void processRadioMessage(const PubSub::Messages::LoRaRXData &data);

    void checkPacketLossResetTimeout();
    void checkTelemetryResponseTimeout();
    void sendTelemetryResponse();
    void updateRadioState();
};