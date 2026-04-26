#pragma once

#include <datalink.h>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstdint>

class CommanderRMModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_UARTSubscriber{PUBSUB_ID(uart_rx)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_UARTPublisher{PUBSUB_ID(uart_tx)};
    PubSub::Subscriber<PubSub::Topics::LoRaRXData> m_RadioSubscriber{PUBSUB_ID(lora_rx)};
    PubSub::Subscriber<PubSub::Topics::LoRaTXAck> m_AckSubscriber{PUBSUB_ID(lora_tx_ack)};
    PubSub::Publisher<PubSub::Topics::LoRaTXData> m_RadioPublisher{PUBSUB_ID(lora_tx)};

    uint8_t m_Sequence;

    void processUARTMessage(const datalink_message_t &msg);
    void processRadioMessage(const PubSub::Topics::LoRaRXData &data);
    void processRadioAck();
};