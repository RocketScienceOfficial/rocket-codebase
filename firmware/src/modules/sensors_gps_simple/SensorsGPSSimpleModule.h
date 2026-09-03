#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <lib/drivers/gps/nmea.h>
#include <hal/uart_driver.h>

#define UART_MAX_BYTES_PER_TICK 256

class SensorsGPSSimpleModule
{
public:
    SensorsGPSSimpleModule(hal_uart_bus_t uartBus)
        : m_UARTBus(uartBus) {}

    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::sensors_simplified_gps_1_topic> m_Publisher;

    const hal_uart_bus_t m_UARTBus;

    uint8_t m_ReceiveFIFOBuffer[UART_MAX_BYTES_PER_TICK];
    char m_CurrentSentence[NMEA_SENTENCE_MAX_LENGTH];
    size_t m_CurrentSentenceLength = 0;

    void parseSentence();
};