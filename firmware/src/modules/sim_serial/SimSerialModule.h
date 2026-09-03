#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimSerialModule
{
public:
    SimSerialModule(uint16_t port) : m_Host(NULL), m_Port(port) {}
    SimSerialModule(const char *host, uint16_t port) : m_Host(host), m_Port(port) {}
    ~SimSerialModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::serial_rx_topic> m_SerialPublisher;
    PubSub::Subscriber<PubSub::Topics::serial_tx_topic> m_SerialSubscriber;

    const char *m_Host;
    const uint16_t m_Port;
    network::TCPSocket m_SerialSocket;

    bool m_Flushed = false;

    void receive();
    void sendIfAvailable();
};