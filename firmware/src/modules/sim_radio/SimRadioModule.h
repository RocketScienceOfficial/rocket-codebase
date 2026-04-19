#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimRadioModule
{
public:
    ~SimRadioModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_RadioPublisher{PUBSUB_ID(radio_rx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_RadioSubscriber{PUBSUB_ID(radio_tx)};

    network::TCPSocket m_RadioSocket;

    void receive();
    void sendIfAvailable();
};