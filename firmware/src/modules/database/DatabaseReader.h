#pragma once

#include "DatabaseMetadataController.h"
#include "DatabaseFlashConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <stdint.h>

template <typename TxTopic, typename ReadyTopic>
class DatabaseReader
{
public:
    DatabaseReader(PubSub::Publisher<TxTopic> &txPub, const DatabaseMetadataController<ReadyTopic> &metadata) : m_TXPublisher(txPub), m_MetadataController(metadata), m_RecoverMode(false), m_Initialized(false), m_Terminated(true) {}

    void update();
    bool isFinished() const;
    void setRecoveryMode(bool enabled);

private:
    PubSub::Publisher<TxTopic> &m_TXPublisher;
    const DatabaseMetadataController<ReadyTopic> &m_MetadataController;

    bool m_RecoverMode;
    bool m_Initialized;
    bool m_Terminated;
    size_t m_CurrentStandingFrameCount;
    size_t m_CurrentSavedFrameCount;
    size_t m_CurrentFrameCount;
    size_t m_CurrentDataOffset;
    bool m_NewSectionInitialized;

    void onInit();
    void onUpdate();
    void onExit();
    void readNext();
    bool isFrameValid(const DatabaseFrameRaw *rawFrame);
    void sendFrame(const DatabaseFrameRaw *rawFrame);
    void handleFaultyFrameRead();
    void handleFaultyFrameRecovery();
};