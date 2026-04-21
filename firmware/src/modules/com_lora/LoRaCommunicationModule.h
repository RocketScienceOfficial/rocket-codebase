#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstdint>

#define LORA_BUFFER_SIZE 512

class LoRaCommunicationModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_Publisher{PUBSUB_ID(uart_tx)};
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_Subscriber{PUBSUB_ID(uart_rx)};

    bool m_Transmitting;
    bool m_DisableNextTransmit;
    uint8_t m_ReceiveBuffer[LORA_BUFFER_SIZE];
    uint8_t m_TransmitBuffer[LORA_BUFFER_SIZE];
    uint8_t m_Sequence;

    void checkIncomingMessages();
    void checkRadio();
    void handleTX();
    void handleRX();
    void setTX();
    void setRX();
};