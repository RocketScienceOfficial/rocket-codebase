#include "DatabaseMetadataController.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <datalink.h>
#include <hal/flash_driver.h>
#include <string.h>

void DatabaseMetadataController::read()
{
    const uint8_t *data;
    hal_flash_read(SECTORS_OFFSET_METADATA * BOARD_FLASH_SECTOR_SIZE, &data);

    const DatabaseMetadataRaw *info = (const DatabaseMetadataRaw *)data;

    if (validateInfo(info))
    {
        m_CurrentMetadata = info->metadata;

        sendReadyNotification();

        LOG_INFO("Database metadata loaded: savedFramesCount=%u, standingFramesCount=%u", m_CurrentMetadata.savedFramesCount, m_CurrentMetadata.standingFramesCount);
    }
    else
    {
        m_CurrentMetadata = {};

        save(m_CurrentMetadata);
    }
}

void DatabaseMetadataController::save(const DatabaseMetadata &metadata)
{
    DatabaseMetadataRaw rawMetadata;
    rawMetadata.magic = DATABASE_METADATA_MAGIC;
    rawMetadata.metadata = metadata;
    rawMetadata.crc = datalink_crc16_mcrf4xx_calculate((const uint8_t *)&rawMetadata, sizeof(rawMetadata) - 2);

    m_CurrentMetadata = metadata;

    uint8_t data[256];
    memset(data, 0xff, sizeof(data));
    memcpy(data, &rawMetadata, sizeof(DatabaseMetadataRaw));

    hal_flash_erase_sectors(SECTORS_OFFSET_METADATA, 1);
    hal_flash_write_pages(SECTORS_OFFSET_METADATA * BOARD_FLASH_SECTOR_SIZE / BOARD_FLASH_PAGE_SIZE, data, 1);

    sendReadyNotification();

    LOG_INFO("Database metadata saved: savedFramesCount=%u, standingFramesCount=%u", metadata.savedFramesCount, metadata.standingFramesCount);
}

void DatabaseMetadataController::sendReadyNotification()
{
    bool ready = m_CurrentMetadata.savedFramesCount + m_CurrentMetadata.standingFramesCount == 0;

    m_ReadyPublisher.publish({.ready = ready});
}

bool DatabaseMetadataController::validateInfo(const DatabaseMetadataRaw *info)
{
    if (info->magic != DATABASE_METADATA_MAGIC)
    {
        return false;
    }

    uint16_t crc = datalink_crc16_mcrf4xx_calculate((const uint8_t *)info, sizeof(DatabaseMetadataRaw) - 2);

    return crc == info->crc;
}