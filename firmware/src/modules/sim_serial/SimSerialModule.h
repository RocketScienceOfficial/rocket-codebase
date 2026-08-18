#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimSerialModule
{
public:
    ~SimSerialModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::serial_rx_topic> m_SerialPublisher;
    PubSub::Subscriber<PubSub::Topics::serial_tx_topic> m_SerialSubscriber;

    network::TCPSocket m_SerialSocket;
    bool m_Flushed;

    void receive();
    void sendIfAvailable();
};