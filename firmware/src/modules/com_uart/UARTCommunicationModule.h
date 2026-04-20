#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstddef>

#define UART_TX_BUFFER_SIZE 512
#define UART_RX_BUFFER_SIZE 512
#define UART_MAX_BYTES_PER_TICK 256
#define UART_MAX_TX_MSG_PER_TICK 4
#define UART_STARTUP_FLUSH_MAX_TRIES 10

class UARTCommunicationModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_Publisher{PUBSUB_ID(uart_rx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_Subscriber{PUBSUB_ID(uart_tx)};

    uint8_t m_SendBuffer[UART_TX_BUFFER_SIZE];
    uint8_t m_ReceiveFIFOBuffer[UART_MAX_BYTES_PER_TICK];
    uint8_t m_ReceiveBuffer[UART_RX_BUFFER_SIZE];
    size_t m_CurrentSendBufferSize;
    size_t m_CurrentReceiveBufferSize;
    bool m_UARTStartFlushed;

    void drainTXBuffer();
    void drainRXBuffer();

    void addToSendQueue(const datalink_message_t &message);
    void flushStartup();
};