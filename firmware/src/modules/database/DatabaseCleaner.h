#pragma once

#include "DatabaseMetadataController.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <stdint.h>

class DatabaseCleaner
{
public:
    DatabaseCleaner(PubSub::Publisher<PubSub::Topics::DatalinkMessage> &txPub, DatabaseMetadataController &metadata) : m_TXPublisher(txPub), m_MetadataController(metadata), m_Initialized(false), m_Terminated(true) {}

    void update();
    bool isFinished() const;

private:
    static constexpr size_t TOTAL_COUNT = SECTORS_COUNT_STANDING_BUFFER + SECTORS_COUNT_DATA;

    PubSub::Publisher<PubSub::Topics::DatalinkMessage> &m_TXPublisher;
    DatabaseMetadataController &m_MetadataController;

    bool m_Initialized;
    bool m_Terminated;
    size_t m_CurrentIndex;

    void onInit();
    void onUpdate();
    void onExit();
};