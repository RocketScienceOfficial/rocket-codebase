#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimLoRaModule
{
public:
    ~SimLoRaModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::LoRaRXData> m_LoRaPublisher{PUBSUB_ID(lora_rx)};
    PubSub::Publisher<PubSub::Topics::LoRaTXAck> m_AckPublisher{PUBSUB_ID(lora_tx_ack)};
    PubSub::Subscriber<PubSub::Topics::LoRaTXData> m_LoRaSubscriber{PUBSUB_ID(lora_tx)};

    network::TCPSocket m_LoRaSocket;
    bool m_Flushed;
    uint8_t m_Sequence;

    void receive();
    void sendIfAvailable();
};