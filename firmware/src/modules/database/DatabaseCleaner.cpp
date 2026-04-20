#include "DatabaseCleaner.h"
#include "DatabaseFlashConfig.h"
#include "modules/common/ModuleLogger.h"
#include <hal/flash_driver.h>

void DatabaseCleaner::update()
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

bool DatabaseCleaner::isFinished() const
{
    return !m_Initialized && m_Terminated;
}

void DatabaseCleaner::onInit()
{
    m_CurrentIndex = 0;

    LOG_INFO("Starting database clearing");
}

void DatabaseCleaner::onUpdate()
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

void DatabaseCleaner::onExit()
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