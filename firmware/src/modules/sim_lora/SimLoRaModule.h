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
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_LoRaPublisher{PUBSUB_ID(uart_tx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_LoRaSubscriber{PUBSUB_ID(uart_rx)};

    network::TCPSocket m_LoRaSocket;
    bool m_Flushed;

    void receive();
    void sendIfAvailable();
};