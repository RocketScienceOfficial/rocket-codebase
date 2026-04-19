#include "DatabaseWriter.h"
#include "modules/common/ModuleLogger.h"
#include <datalink.h>
#include <hal/flash_driver.h>
#include <hal/time_driver.h>
#include <string.h>

static void _reverse_buff(DatabaseFrameRaw *buffer, size_t start, size_t end)
{
    while (start < end)
    {
        DatabaseFrameRaw tmp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = tmp;

        start++;
        end--;
    }
}

void DatabaseWriter::saveStandingFrame(const DatabaseFrame &frame)
{
    DatabaseFrameRaw rawFrame = getFrame(frame);

    m_StandingBuffer[m_StandingBufferIndex++] = rawFrame;

    if (m_StandingBufferIndex >= STANDING_BUFFER_LENGTH)
    {
        m_StandingBufferIndex = 0;
    }

    if (m_StandingBufferLength < STANDING_BUFFER_LENGTH)
    {
        m_StandingBufferLength++;
    }
}

void DatabaseWriter::saveFrame(const DatabaseFrame &frame)
{
    if (!canSaveData())
    {
        return;
    }

    DatabaseFrameRaw rawFrame = getFrame(frame);
    uint8_t *data = (uint8_t *)&rawFrame;
    size_t len = sizeof(rawFrame);

    m_SavedFramesCount++;

    if (m_SaveBufferSize + len <= sizeof(m_SaveBuffer))
    {
        memcpy(m_SaveBuffer + m_SaveBufferSize, data, len);
        m_SaveBufferSize += len;
    }
    else
    {
        int tmpLen = (int)sizeof(m_SaveBuffer) - (int)m_SaveBufferSize;

        if (tmpLen > 0)
        {
            memcpy(m_SaveBuffer + m_SaveBufferSize, data, tmpLen);

            m_SaveBufferSize += tmpLen;
        }

        size_t pages = m_SaveBufferSize / BOARD_FLASH_PAGE_SIZE;

        hal_flash_write_pages(SECTORS_OFFSET_DATA * BOARD_FLASH_SECTOR_SIZE / BOARD_FLASH_PAGE_SIZE + m_SaveFlashOffsetPages, m_SaveBuffer, pages);

        m_SaveFlashOffsetPages += pages;

        memcpy(m_SaveBuffer, data + tmpLen, len - tmpLen);
        m_SaveBufferSize = len - tmpLen;
    }
}

void DatabaseWriter::flush()
{
    flushData();
    flushStandingBuffer();

    DatabaseMetadata metadata = {
        .savedFramesCount = m_SavedFramesCount,
        .standingFramesCount = m_StandingBufferLength,
    };
    m_MetadataController.save(metadata);
}

DatabaseFrameRaw DatabaseWriter::getFrame(const DatabaseFrame &frame)
{
    DatabaseFrameRaw raw;
    raw.magic = DATABASE_FRAME_MAGIC;
    raw.frame = frame;
    raw.crc = datalink_crc16_mcrf4xx_calculate((const uint8_t *)&raw, sizeof(raw) - 2);

    return raw;
}

bool DatabaseWriter::canSaveData() const
{
    return m_SaveFlashOffsetPages + sizeof(m_SaveBuffer) / BOARD_FLASH_PAGE_SIZE <= SECTORS_COUNT_DATA * BOARD_FLASH_SECTOR_SIZE / BOARD_FLASH_PAGE_SIZE;
}

void DatabaseWriter::flushData()
{
    if (canSaveData())
    {
        if (m_SaveBufferSize > 0)
        {
            memset(m_SaveBuffer + m_SaveBufferSize, 0, sizeof(m_SaveBuffer) - m_SaveBufferSize);

            hal_flash_write_pages(SECTORS_OFFSET_DATA * BOARD_FLASH_SECTOR_SIZE / BOARD_FLASH_PAGE_SIZE + m_SaveFlashOffsetPages, m_SaveBuffer, sizeof(m_SaveBuffer) / BOARD_FLASH_PAGE_SIZE);
        }

        OBC_INFO("Flash save buffer has been flushed. %d bytes were written", m_SaveBufferSize);

        m_SaveBufferSize = 0;
    }
}

void DatabaseWriter::flushStandingBuffer()
{
    if (m_StandingBufferIndex != 0)
    {
        _reverse_buff(m_StandingBuffer, 0, m_StandingBufferIndex - 1);
        _reverse_buff(m_StandingBuffer, m_StandingBufferIndex, STANDING_BUFFER_LENGTH - 1);
        _reverse_buff(m_StandingBuffer, 0, STANDING_BUFFER_LENGTH - 1);
    }

    uint8_t *data = (uint8_t *)m_StandingBuffer;
    size_t pages = sizeof(m_StandingBuffer) / BOARD_FLASH_PAGE_SIZE;

    hal_flash_write_pages(SECTORS_OFFSET_STANDING_BUFFER * BOARD_FLASH_SECTOR_SIZE / BOARD_FLASH_PAGE_SIZE, data, pages);

    OBC_INFO("Standing buffer has been flushed. %d frames (%d bytes) were written", m_StandingBufferLength, m_StandingBufferLength * sizeof(DatabaseFrameRaw));
}