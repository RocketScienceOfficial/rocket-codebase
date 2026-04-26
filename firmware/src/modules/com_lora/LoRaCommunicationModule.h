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
    PubSub::Publisher<PubSub::Topics::LoRaRXData> m_RXPublisher{PUBSUB_ID(lora_rx)};
    PubSub::Publisher<PubSub::Topics::LoRaTXAck> m_AckPublisher{PUBSUB_ID(lora_tx_ack)};
    PubSub::Subscriber<PubSub::Topics::LoRaTXData> m_Subscriber{PUBSUB_ID(lora_tx)};

    bool m_Transmitting;
    uint8_t m_ReceiveBuffer[LORA_BUFFER_SIZE];
    uint8_t m_TransmitBuffer[LORA_BUFFER_SIZE];

    void checkIncomingMessages();
    void checkRadio();
    void handleTX();
    void handleRX();
    void setTX();
    void setRX();
};