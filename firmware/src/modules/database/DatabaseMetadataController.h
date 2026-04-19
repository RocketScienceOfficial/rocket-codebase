#pragma once

#include "DatabaseFlashConfig.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>

class DatabaseMetadataController
{
public:
    DatabaseMetadataController(PubSub::Publisher<PubSub::Topics::DatabaseReady> &readyPub) : m_ReadyPublisher(readyPub) {}

    void read();
    void save(const DatabaseMetadata &metadata);

    const DatabaseMetadata &getMetadata() const { return m_CurrentMetadata; }

private:
    PubSub::Publisher<PubSub::Topics::DatabaseReady> &m_ReadyPublisher;

    DatabaseMetadata m_CurrentMetadata;

    void sendReadyNotification();
    bool validateInfo(const DatabaseMetadataRaw *info);
};