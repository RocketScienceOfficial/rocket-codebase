#include "DatabaseCleaner.h"
#include "DatabaseFlashConfig.h"
#include "modules/common/ModuleLogger.h"
#include <hal/flash_driver.h>

template <typename TxTopic, typename ReadyTopic>
void DatabaseCleaner<TxTopic, ReadyTopic>::update()
{
    if (!m_Initialized)
    {
        onInit();

        m_Initialized = true;
        m_Terminated = false;
    }
    else if (!m_Terminated)
    {
        onUpdate();
    }
    else
    {
        onExit();

        m_Initialized = false;
        m_Terminated = true;
    }
}

template <typename TxTopic, typename ReadyTopic>
bool DatabaseCleaner<TxTopic, ReadyTopic>::isFinished() const
{
    return !m_Initialized && m_Terminated;
}

template <typename TxTopic, typename ReadyTopic>
void DatabaseCleaner<TxTopic, ReadyTopic>::onInit()
{
    m_CurrentIndex = 0;

    LOG_INFO("Starting database clearing");
}

template <typename TxTopic, typename ReadyTopic>
void DatabaseCleaner<TxTopic, ReadyTopic>::onUpdate()
{
    if (m_CurrentIndex == TOTAL_COUNT)
    {
        m_Terminated = true;

        return;
    }

    hal_flash_erase_sectors(SECTORS_OFFSET_STANDING_BUFFER + m_CurrentIndex, 1);

    float progress = (m_CurrentIndex + 1) * 100 / (float)TOTAL_COUNT;

    clear_progress_info payload;
    payload.percentage = (uint8_t)progress;

    datalink_message_t msg;
    datalink_pack_clear_progress_info(&payload, &msg);

    m_TXPublisher.publish(msg);

    m_CurrentIndex++;
}

template <typename TxTopic, typename ReadyTopic>
void DatabaseCleaner<TxTopic, ReadyTopic>::onExit()
{
    DatabaseMetadata metadata = {
        .savedFramesCount = 0,
        .standingFramesCount = 0,
    };
    m_MetadataController.save(metadata);

    datalink_message_t msg;
    datalink_pack_clear_finish(&msg);

    m_TXPublisher.publish(msg);

    LOG_INFO("Database cleared");
}

template class DatabaseCleaner<PubSub::Topics::database_tx_topic, PubSub::Topics::database_ready_topic>;
