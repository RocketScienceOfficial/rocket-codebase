#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <hal/uart_driver.h>
#include <cstddef>

#define UART_TX_BUFFER_SIZE 512
#define UART_RX_BUFFER_SIZE 512
#define UART_MAX_BYTES_PER_TICK 256
#define UART_MAX_TX_MSG_PER_TICK 4
#define UART_STARTUP_FLUSH_MAX_TRIES 10

class UARTCommunicationModule
{
public:
    UARTCommunicationModule(hal_uart_bus_t uartBus)
        : m_UARTBus(uartBus) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::uart_rx_topic> m_Publisher;
    PubSub::Subscriber<PubSub::Topics::uart_tx_topic> m_Subscriber;

    const hal_uart_bus_t m_UARTBus;

    uint8_t m_SendBuffer[UART_TX_BUFFER_SIZE];
    uint8_t m_ReceiveFIFOBuffer[UART_MAX_BYTES_PER_TICK];
    uint8_t m_ReceiveBuffer[UART_RX_BUFFER_SIZE];
    size_t m_CurrentSendBufferSize = 0;
    size_t m_CurrentReceiveBufferSize = 0;
    bool m_UARTStartFlushed = false;

    void drainTXBuffer();
    void drainRXBuffer();

    void addToSendQueue(const datalink_message_t &message);
    void flushStartup();
};