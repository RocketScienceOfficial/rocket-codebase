#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimUARTModule
{
public:
    ~SimUARTModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_UARTPublisher{PUBSUB_ID(uart_rx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_UARTSubscriber{PUBSUB_ID(uart_tx)};

    network::TCPSocket m_UARTSocket;
    bool m_Flushed;

    void receive();
    void sendIfAvailable();
};