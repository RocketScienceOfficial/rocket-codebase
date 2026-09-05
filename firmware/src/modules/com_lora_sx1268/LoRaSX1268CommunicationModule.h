#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <RadioLib.h>
#include <lib/radio/RadioLibHALPort.h>
#include <cstdint>

#define LORA_BUFFER_SIZE 512

class LoRaSX1268CommunicationModule
{
public:
    LoRaSX1268CommunicationModule(hal_spi_bus_t spi,
                                  hal_gpio_pin_t cs,
                                  hal_gpio_pin_t dio1,
                                  hal_gpio_pin_t reset,
                                  hal_gpio_pin_t busy,
                                  uint32_t frequency_mhz,
                                  uint32_t bandwidth_khz,
                                  uint8_t spreadingFactor,
                                  uint8_t transmitPower,
                                  hal_gpio_pin_t txen,
                                  hal_gpio_pin_t rxen,
                                  uint8_t srcId,
                                  uint8_t dstId)
        : m_RadioHAL(spi),
          m_RadioModule(&m_RadioHAL, cs, dio1, reset, busy),
          m_Radio(&m_RadioModule),
          m_RadioFrequencyMHz(frequency_mhz),
          m_RadioBandwidthKHz(bandwidth_khz),
          m_RadioSpreadingFactor(spreadingFactor),
          m_RadioTransmitPower(transmitPower),
          m_TxenPin(txen),
          m_RxenPin(rxen),
          m_srcId(srcId),
          m_dstId(dstId) {}

    void init();
    void run();

private:
    // API
    PubSub::Publisher<PubSub::Topics::lora_rx_topic> m_RXPublisher;
    PubSub::Publisher<PubSub::Topics::lora_tx_ack_topic> m_AckPublisher;
    PubSub::Subscriber<PubSub::Topics::lora_tx_topic> m_Subscriber;

    // Radio config
    RadioLibHALPort m_RadioHAL;
    Module m_RadioModule;
    SX1268 m_Radio;
    const uint32_t m_RadioFrequencyMHz;
    const uint32_t m_RadioBandwidthKHz;
    const uint8_t m_RadioSpreadingFactor;
    const uint8_t m_RadioTransmitPower;
    const hal_gpio_pin_t m_TxenPin;
    const hal_gpio_pin_t m_RxenPin;
    const uint8_t m_srcId;
    const uint8_t m_dstId;

    // State
    bool m_Transmitting = false;
    uint8_t m_ReceiveBuffer[LORA_BUFFER_SIZE];
    uint8_t m_TransmitBuffer[LORA_BUFFER_SIZE];

    // Methods
    void checkIncomingMessages();
    void checkRadio();
    void handleTX();
    void handleRX();
    void setTX();
    void setRX();
};