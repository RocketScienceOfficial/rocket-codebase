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
    PubSub::Subscriber<PubSub::Topics::uart_rx_topic> m_UARTSubscriber;
    PubSub::Publisher<PubSub::Topics::uart_tx_topic> m_UARTPublisher;
    PubSub::Subscriber<PubSub::Topics::lora_rx_topic> m_RadioSubscriber;
    PubSub::Subscriber<PubSub::Topics::lora_tx_ack_topic> m_AckSubscriber;
    PubSub::Publisher<PubSub::Topics::lora_tx_topic> m_RadioPublisher;

    uint8_t m_Sequence;

    void processUARTMessage(const datalink_message_t &msg);
    void processRadioMessage(const PubSub::Messages::LoRaRXData &data);
    void processRadioAck();
};