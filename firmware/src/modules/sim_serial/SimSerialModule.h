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
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_SerialPublisher{PUBSUB_ID(serial_rx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_SerialSubscriber{PUBSUB_ID(serial_tx)};

    network::TCPSocket m_SerialSocket;

    void receive();
    void sendIfAvailable();
};