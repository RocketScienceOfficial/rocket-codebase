#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstddef>

#define SERIAL_BUFFER_SIZE 512
#define SERIAL_MAX_TX_MSG_PER_TICK 4
#define SERIAL_MAX_BYTES_PER_TICK 64
#define SERIAL_STARTUP_FLUSH_MAX_BYTES 64

class SerialCommunicationModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_Publisher{PUBSUB_ID(serial_rx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_Subscriber{PUBSUB_ID(serial_tx)};

    uint8_t m_SendBuffer[SERIAL_BUFFER_SIZE];
    uint8_t m_ReceiveBuffer[SERIAL_BUFFER_SIZE];
    size_t m_CurrentReceiveBufferSize;
    bool m_SerialStartFlushed = false;

    void drainTXBuffer();
    void drainRXBuffer();

    void send(const datalink_message_t &message);
    void flushStartup();
};