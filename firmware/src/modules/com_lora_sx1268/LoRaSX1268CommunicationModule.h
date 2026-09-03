#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <RadioLib.h>
#include <cstdint>

#define LORA_BUFFER_SIZE 512

class LoRaSX1268CommunicationModule
{
public:
    LoRaSX1268CommunicationModule()
        : m_RadioHAL(CFG_LORA_SPI, CFG_LORA_SPI_MISO_PIN, CFG_LORA_SPI_MOSI_PIN, CFG_LORA_SPI_SCK_PIN),
          m_RadioModule(&m_RadioHAL, CFG_LORA_PIN_CS, CFG_LORA_PIN_DIO1, CFG_LORA_PIN_RESET, CFG_LORA_PIN_BUSY),
          m_Radio(&m_RadioModule)
    {
    }

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::lora_rx_topic> m_RXPublisher;
    PubSub::Publisher<PubSub::Topics::lora_tx_ack_topic> m_AckPublisher;
    PubSub::Subscriber<PubSub::Topics::lora_tx_topic> m_Subscriber;

    RadioLibHALPort m_RadioHAL;
    Module m_RadioModule;
    SX1268 m_Radio;

    bool m_Transmitting = false;
    uint8_t m_ReceiveBuffer[LORA_BUFFER_SIZE];
    uint8_t m_TransmitBuffer[LORA_BUFFER_SIZE];

    void checkIncomingMessages();
    void checkRadio();
    void handleTX();
    void handleRX();
    void setTX();
    void setRX();
};