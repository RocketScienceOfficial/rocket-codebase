#pragma once

#include "DatabaseMetadataController.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <stdint.h>

class DatabaseCleaner
{
public:
    DatabaseCleaner(PubSub::Publisher<PubSub::Topics::database_tx_topic> &txPub, DatabaseMetadataController &metadata) : m_TXPublisher(txPub), m_MetadataController(metadata), m_Initialized(false), m_Terminated(true) {}

    void update();
    bool isFinished() const;

private:
    static constexpr size_t TOTAL_COUNT = SECTORS_COUNT_STANDING_BUFFER + SECTORS_COUNT_DATA;

    PubSub::Publisher<PubSub::Topics::database_tx_topic> &m_TXPublisher;
    DatabaseMetadataController &m_MetadataController;

    bool m_Initialized = false;
    bool m_Terminated = false;
    size_t m_CurrentIndex = 0;

    void onInit();
    void onUpdate();
    void onExit();
};