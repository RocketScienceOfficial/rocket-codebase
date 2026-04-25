#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <lib/drivers/gps/nmea.h>

#define UART_MAX_BYTES_PER_TICK 256

class SimpleGPSModule
{
public:
    void init();
    void run();

private:
    PubSub::Publisher<PubSub::Topics::SensorsSimplifiedGPS> m_Publisher{PUBSUB_ID(sensors_simplified_gps_1)};

    uint8_t m_ReceiveFIFOBuffer[UART_MAX_BYTES_PER_TICK];
    char m_CurrentSentence[NMEA_SENTENCE_MAX_LENGTH];
    size_t m_CurrentSentenceLength;

    void parseSentence();
};