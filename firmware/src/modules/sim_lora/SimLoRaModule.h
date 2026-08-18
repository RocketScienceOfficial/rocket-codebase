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
    PubSub::Publisher<PubSub::Topics::lora_rx_topic> m_LoRaPublisher;
    PubSub::Publisher<PubSub::Topics::lora_tx_ack_topic> m_AckPublisher;
    PubSub::Subscriber<PubSub::Topics::lora_tx_topic> m_LoRaSubscriber;

    network::TCPSocket m_LoRaSocket;
    bool m_Flushed;
    uint8_t m_Sequence;

    void receive();
    void sendIfAvailable();
};