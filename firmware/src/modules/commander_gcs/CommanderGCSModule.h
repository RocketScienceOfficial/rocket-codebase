#pragma once

#include <datalink.h>
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <cstdint>

class CommanderGCSModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::DatalinkMessage> m_SerialSubscriber{PUBSUB_ID(serial_rx)};
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> m_SerialPublisher{PUBSUB_ID(serial_tx)};
    PubSub::Publisher<PubSub::Topics::GCSCommanderTimeout> m_CommandTimeoutPublisher{PUBSUB_ID(gcs_commander_timeout)};

    uint8_t m_CurrentCMD;
    uint8_t m_CurrentCommandSeq;
    uint8_t m_RemoteCommandSeq;
    bool m_CommandActive;
    uint32_t m_CommandStartTime;
    uint8_t m_ElapsedTimeSec;

    void processSerialMessage(const datalink_message_t &msg);
    void processNewRadioData(uint8_t seq, uint8_t status);
    void handleCommandElapsedTime();

    void set(uint8_t cmd);
    void reset();
    void ack(bool success);
    void nack();
};