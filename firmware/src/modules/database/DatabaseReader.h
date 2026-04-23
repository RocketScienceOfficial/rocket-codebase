#pragma once

#include "DatabaseMetadataController.h"
#include "DatabaseFlashConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <stdint.h>

class DatabaseReader
{
public:
    DatabaseReader(PubSub::Publisher<PubSub::Topics::DatalinkMessage> &txPub, const DatabaseMetadataController &metadata) : m_TXPublisher(txPub), m_MetadataController(metadata), m_RecoverMode(false), m_Initialized(false), m_Terminated(true) {}

    void update();
    bool isFinished() const;
    void setRecoveryMode(bool enabled);

private:
    PubSub::Publisher<PubSub::Topics::DatalinkMessage> &m_TXPublisher;
    const DatabaseMetadataController &m_MetadataController;

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