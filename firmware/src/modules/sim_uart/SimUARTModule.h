#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <network/TCPSocket.h>

class SimUARTModule
{
public:
    SimUARTModule(uint16_t port) : m_Host(NULL), m_Port(port) {}
    SimUARTModule(const char *host, uint16_t port) : m_Host(host), m_Port(port) {}
    ~SimUARTModule();

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::uart_rx_topic> m_UARTPublisher;
    PubSub::Subscriber<PubSub::Topics::uart_tx_topic> m_UARTSubscriber;

    const char *m_Host;
    const uint16_t m_Port;
    network::TCPSocket m_UARTSocket;

    bool m_Flushed = false;

    void receive();
    void sendIfAvailable();
};