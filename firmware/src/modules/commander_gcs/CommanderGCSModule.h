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

    uint8_t m_RXSequence = 0;
    uint8_t m_TXSequence = 0;
    uint32_t m_PacketsLost = 0;
    uint32_t m_RadioRX = 0;
    uint32_t m_RadioTmpRX = 0;
    uint32_t m_RadioTX = 0;
    uint32_t m_ResponseStartTime = 0;
    uint32_t m_RadioTmpRXStartTime = 0;

    void processSerialMessage(const datalink_message_t &msg);
    void processRadioMessage(const PubSub::Messages::LoRaRXData &data);

    void checkPacketLossResetTimeout();
    void checkTelemetryResponseTimeout();
    void sendTelemetryResponse();
    void updateRadioState();
};