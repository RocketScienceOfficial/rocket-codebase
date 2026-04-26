#pragma once

#include <cstdint>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>

class GCSCommandHandler
{
public:
    GCSCommandHandler(PubSub::Publisher<PubSub::Topics::DatalinkMessage> &serialPublisher, PubSub::Publisher<PubSub::Topics::GCSCommanderTimeout> &commandTimeoutPublisher)
        : m_SerialPublisher(serialPublisher), m_CommandTimeoutPublisher(commandTimeoutPublisher) {}

    void update();
    void set(uint8_t cmd);
    void onNewSequence(uint8_t seq, uint8_t status);

    uint8_t getCurrentCommand() const { return m_CurrentCMD; }
    uint8_t getCurrentSequence() const { return m_CurrentCommandSeq; }

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> &m_SerialPublisher;
    PubSub::Publisher<PubSub::Topics::GCSCommanderTimeout> &m_CommandTimeoutPublisher;

    uint8_t m_CurrentCMD;
    uint8_t m_CurrentCommandSeq;
    uint8_t m_RemoteCommandSeq;
    bool m_CommandActive;
    uint32_t m_CommandStartTime;
    uint8_t m_ElapsedTimeSec;

    void handleCommandElapsedTime();

    void reset();
    void ack(bool success);
    void nack();
};