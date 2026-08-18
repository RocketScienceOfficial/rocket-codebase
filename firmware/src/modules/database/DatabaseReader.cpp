#include "DatabaseReader.h"
#include "modules/common/ModuleLogger.h"
#include <datalink.h>
#include <hal/flash_driver.h>

void DatabaseReader::update()
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

bool DatabaseReader::isFinished() const
{
    return !m_Initialized && m_Terminated;
}

void DatabaseReader::setRecoveryMode(bool enabled)
{
    if (m_RecoverMode != enabled && isFinished())
    {
        m_RecoverMode = enabled;
    }
}

void DatabaseReader::onInit()
{
    if (!m_RecoverMode)
    {
        m_CurrentStandingFrameCount = m_MetadataController.getMetadata().standingFramesCount;
        m_CurrentSavedFrameCount = m_MetadataController.getMetadata().savedFramesCount;
        m_CurrentFrameCount = m_CurrentStandingFrameCount + m_CurrentSavedFrameCount;

        saved_data_chunk_size payload;
        payload.size = (uint32_t)m_CurrentFrameCount;

        datalink_message_t msg;
        datalink_pack_saved_data_chunk_size(&payload, &msg);

        m_TXPublisher.publish(msg);

        LOG_INFO("Beginning data read. Attempting to read %d frames", m_CurrentFrameCount);
    }
    else
    {
        m_CurrentStandingFrameCount = DATA_RECOVERY_MAX_FRAMES;
        m_CurrentSavedFrameCount = DATA_RECOVERY_MAX_FRAMES;
        m_CurrentFrameCount = m_CurrentStandingFrameCount + m_CurrentSavedFrameCount;

        LOG_INFO("Beginning data recovery. Attempting to read up to %d frames", m_CurrentFrameCount);
    }

    m_NewSectionInitialized = false;
}

void DatabaseReader::onUpdate()
{
    if (m_CurrentStandingFrameCount > 0)
    {
        m_CurrentStandingFrameCount--;

        if (!m_NewSectionInitialized)
        {
            m_CurrentDataOffset = SECTORS_OFFSET_STANDING_BUFFER * BOARD_FLASH_SECTOR_SIZE;
            m_NewSectionInitialized = true;
        }

        readNext();

        if (m_CurrentStandingFrameCount == 0)
        {
            m_NewSectionInitialized = false;
        }
    }
    else if (m_CurrentSavedFrameCount > 0)
    {
        m_CurrentSavedFrameCount--;

        if (!m_NewSectionInitialized)
        {
            m_CurrentDataOffset = SECTORS_OFFSET_DATA * BOARD_FLASH_SECTOR_SIZE;
            m_NewSectionInitialized = true;
        }

        readNext();

        if (m_CurrentSavedFrameCount == 0)
        {
            m_NewSectionInitialized = false;
        }
    }
    else
    {
        m_Terminated = true;

        return;
    }
}

void DatabaseReader::onExit()
{
    datalink_message_t msg;

    if (!m_RecoverMode)
    {
        datalink_pack_read_finish(&msg);
    }
    else
    {
        datalink_pack_recovery_finish(&msg);
    }

    m_TXPublisher.publish(msg);

    LOG_INFO("Data read has finished");
}

void DatabaseReader::readNext()
{
    DatabaseFrameRaw frame;
    hal_flash_read(m_CurrentDataOffset, (uint8_t *)&frame, sizeof(frame));
    m_CurrentDataOffset += sizeof(frame);

    if (isFrameValid(&frame))
    {
        sendFrame(&frame);
    }
    else
    {
        LOG_WARN("Read invalid frame");

        if (!m_RecoverMode)
        {
            handleFaultyFrameRead();
        }
        else
        {
            handleFaultyFrameRecovery();
        }
    }
}

bool DatabaseReader::isFrameValid(const DatabaseFrameRaw *rawFrame)
{
    if (rawFrame->magic != DATABASE_FRAME_MAGIC)
    {
        return false;
    }

    uint16_t crc = datalink_crc16_mcrf4xx_calculate((const uint8_t *)rawFrame, sizeof(DatabaseFrameRaw) - 2);

    return crc == rawFrame->crc;
}

void DatabaseReader::sendFrame(const DatabaseFrameRaw *rawFrame)
{
    const DatabaseFrame *frame = &rawFrame->frame;

    saved_data_chunk_obc payload;
    payload.dt = frame->dt_us;
    payload.accRawX = frame->accRaw.x;
    payload.accRawY = frame->accRaw.y;
    payload.accRawZ = frame->accRaw.z;
    payload.gyroRawX = frame->gyroRaw.x;
    payload.gyroRawY = frame->gyroRaw.y;
    payload.gyroRawZ = frame->gyroRaw.z;
    payload.magRawX = frame->magRaw.x;
    payload.magRawY = frame->magRaw.y;
    payload.magRawZ = frame->magRaw.z;
    payload.lat = frame->gpsPos.lat;
    payload.lon = frame->gpsPos.lon;
    payload.alt = frame->gpsPos.alt;
    payload.gpsData = frame->gpsData;
    payload.pressure = frame->pressure;
    payload.velN = frame->velNED.x;
    payload.velE = frame->velNED.y;
    payload.velD = frame->velNED.z;
    payload.posN = frame->posNED.x;
    payload.posE = frame->posNED.y;
    payload.posD = frame->posNED.z;
    payload.qw = frame->qNED.w;
    payload.qx = frame->qNED.x;
    payload.qy = frame->qNED.y;
    payload.qz = frame->qNED.z;
    payload.smState = frame->smState;
    payload.batteryVoltage100 = frame->batteryVoltage100;
    payload.ignFlags = frame->ignFlags;

    datalink_message_t msg;
    datalink_pack_saved_data_chunk_obc(&payload, &msg);

    m_TXPublisher.publish(msg);
}

void DatabaseReader::handleFaultyFrameRead()
{
    m_CurrentFrameCount--;

    saved_data_chunk_size payload;
    payload.size = (uint32_t)m_CurrentFrameCount;

    datalink_message_t msg;
    datalink_pack_saved_data_chunk_size(&payload, &msg);

    m_TXPublisher.publish(msg);
}

void DatabaseReader::handleFaultyFrameRecovery()
{
    if (m_CurrentStandingFrameCount > 0)
    {
        m_CurrentStandingFrameCount = 0;
    }
    else
    {
        m_CurrentSavedFrameCount = 0;
    }
}