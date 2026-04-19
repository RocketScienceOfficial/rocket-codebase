#pragma once

#include "DatabaseMetadataController.h"
#include "DatabaseFlashConfig.h"
#include <cstddef>

class DatabaseWriter
{
public:
    DatabaseWriter(DatabaseMetadataController &metadata) : m_MetadataController(metadata), m_SaveBufferSize(0), m_SaveFlashOffsetPages(0), m_SavedFramesCount(0), m_StandingBufferLength(0), m_StandingBufferIndex(0) {}

    void saveStandingFrame(const DatabaseFrame &frame);
    void saveFrame(const DatabaseFrame &frame);
    void flush();

private:
    DatabaseFrameRaw getFrame(const DatabaseFrame &frame);
    bool canSaveData() const;
    void flushData();
    void flushStandingBuffer();

    DatabaseMetadataController &m_MetadataController;

    uint8_t m_SaveBuffer[BOARD_FLASH_PAGE_SIZE];
    size_t m_SaveBufferSize;
    size_t m_SaveFlashOffsetPages;
    size_t m_SavedFramesCount;

    DatabaseFrameRaw m_StandingBuffer[STANDING_BUFFER_LENGTH];
    size_t m_StandingBufferLength;
    size_t m_StandingBufferIndex;
};