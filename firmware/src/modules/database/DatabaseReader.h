#pragma once

#include "DatabaseMetadataController.h"
#include "DatabaseFlashConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <stdint.h>

class DatabaseReader
{
public:
    DatabaseReader(PubSub::Publisher<PubSub::Topics::database_tx_topic> &txPub, const DatabaseMetadataController &metadata) : m_TXPublisher(txPub), m_MetadataController(metadata), m_RecoverMode(false), m_Initialized(false), m_Terminated(true) {}

    void update();
    bool isFinished() const;
    void setRecoveryMode(bool enabled);

private:
    PubSub::Publisher<PubSub::Topics::database_tx_topic> &m_TXPublisher;
    const DatabaseMetadataController &m_MetadataController;

    bool m_RecoverMode = false;
    bool m_Initialized = false;
    bool m_Terminated = false;
    size_t m_CurrentStandingFrameCount = 0;
    size_t m_CurrentSavedFrameCount = 0;
    size_t m_CurrentFrameCount = 0;
    size_t m_CurrentDataOffset = 0;
    bool m_NewSectionInitialized = false;

    void onInit();
    void onUpdate();
    void onExit();
    void readNext();
    bool isFrameValid(const DatabaseFrameRaw *rawFrame);
    void sendFrame(const DatabaseFrameRaw *rawFrame);
    void handleFaultyFrameRead();
    void handleFaultyFrameRecovery();
};