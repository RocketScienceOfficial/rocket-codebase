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
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_SerialSubscriber{PUBSUB_ID(serial_rx)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_SerialPublisher{PUBSUB_ID(serial_tx)};
    PubSub::Subscriber<PubSub::Topics::LoRaRXData> m_RadioSubscriber{PUBSUB_ID(lora_rx)};
    PubSub::Publisher<PubSub::Topics::LoRaTXData> m_RadioPublisher{PUBSUB_ID(lora_tx)};

    PubSub::Subscriber<PubSub::Topics::SensorsSimplifiedGPS> m_GPSSubscriber{PUBSUB_ID(sensors_simplified_gps_1)};

    PubSub::Publisher<PubSub::Topics::GCSCommanderTimeout> m_CommandTimeoutPublisher{PUBSUB_ID(gcs_commander_timeout)};
    PubSub::Publisher<PubSub::Topics::GCSRadioState> m_RadioStatePublisher{PUBSUB_ID(gcs_radio_state)};

    GCSCommandHandler m_CommandHandler;

    uint8_t m_RXSequence;
    uint8_t m_TXSequence;
    uint32_t m_PacketsLost;
    uint32_t m_RadioRX;
    uint32_t m_RadioTmpRX;
    uint32_t m_RadioTX;
    uint32_t m_ResponseStartTime;
    uint32_t m_RadioTmpRXStartTime;

    void processSerialMessage(const datalink_message_t &msg);
    void processRadioMessage(const PubSub::Topics::LoRaRXData &data);

    void checkPacketLossResetTimeout();
    void checkTelemetryResponseTimeout();
    void sendTelemetryResponse();
    void updateRadioState();
};