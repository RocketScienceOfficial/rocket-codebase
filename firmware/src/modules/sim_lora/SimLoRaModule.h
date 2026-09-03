#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimLoRaModule
{
public:
    SimLoRaModule(uint16_t port) : m_Host(NULL), m_Port(port) {}
    SimLoRaModule(const char *host, uint16_t port) : m_Host(host), m_Port(port) {}
    ~SimLoRaModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::lora_rx_topic> m_LoRaPublisher;
    PubSub::Publisher<PubSub::Topics::lora_tx_ack_topic> m_AckPublisher;
    PubSub::Subscriber<PubSub::Topics::lora_tx_topic> m_LoRaSubscriber;

    const char *m_Host;
    const uint16_t m_Port;
    network::TCPSocket m_LoRaSocket;

    bool m_Flushed = false;
    uint8_t m_Sequence = 0;

    void receive();
    void sendIfAvailable();
};