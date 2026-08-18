#pragma once

#include "DatabaseMetadataController.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <stdint.h>

template <typename TxTopic, typename ReadyTopic>
class DatabaseCleaner
{
public:
    DatabaseCleaner(PubSub::Publisher<TxTopic> &txPub, DatabaseMetadataController<ReadyTopic> &metadata) : m_TXPublisher(txPub), m_MetadataController(metadata), m_Initialized(false), m_Terminated(true) {}

    void update();
    bool isFinished() const;

private:
    static constexpr size_t TOTAL_COUNT = SECTORS_COUNT_STANDING_BUFFER + SECTORS_COUNT_DATA;

    PubSub::Publisher<TxTopic> &m_TXPublisher;
    DatabaseMetadataController<ReadyTopic> &m_MetadataController;

    bool m_Initialized;
    bool m_Terminated;
    size_t m_CurrentIndex;

    void onInit();
    void onUpdate();
    void onExit();
};