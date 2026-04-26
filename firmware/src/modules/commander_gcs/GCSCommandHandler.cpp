#include "GCSCommandHandler.h"
#include "modules/common/ModuleLogger.h"
#include <datalink.h>
#include <osal/systime.h>
#include <lib/debug/sys_assert.h>

#define CMD_TIMEOUT_MS 10000

void GCSCommandHandler::update()
{
    if (m_CommandActive)
    {
        if (osal_systime_get_ms() - m_CommandStartTime > CMD_TIMEOUT_MS)
        {
            LOG_WARN("Command %d timed out", m_CurrentCMD);
            nack();
            return;
        }

        handleCommandElapsedTime();
    }
}

void GCSCommandHandler::set(uint8_t cmd)
{
    if (m_CommandActive)
    {
        LOG_WARN("Received command %d while command %d is active, rejecting", cmd, m_CurrentCMD);
        return;
    }

    m_CurrentCMD = cmd;
    m_CurrentCommandSeq = (m_CurrentCommandSeq + 1) % 256;
    m_CommandStartTime = osal_systime_get_ms();
    m_CommandActive = true;

    LOG_INFO("Set command %d (seq %d)", cmd, m_CurrentCommandSeq);
}

void GCSCommandHandler::onNewSequence(uint8_t seq, uint8_t status)
{
    m_RemoteCommandSeq = seq;

    LOG_INFO("New packet stats:  Current Seq: %d Remote Seq: %d Status: %d", m_CurrentCommandSeq, m_RemoteCommandSeq, status);

    if (m_CommandActive && m_RemoteCommandSeq == m_CurrentCommandSeq)
    {
        if (status == DATALINK_TELEMETRY_CMD_STATUS_PENDING)
        {
            m_CurrentCMD = DATALINK_TELEMETRY_CMD_NONE;
        }
        else
        {
            ack(status == DATALINK_TELEMETRY_CMD_STATUS_SUCCESS);
        }
    }
    else if (!m_CommandActive && m_RemoteCommandSeq != m_CurrentCommandSeq)
    {
        nack();
    }
}

void GCSCommandHandler::handleCommandElapsedTime()
{
    uint32_t diff = osal_systime_get_ms() - m_CommandStartTime;
    int elapsedTime = (CMD_TIMEOUT_MS - (int)diff) / 1000;
    int nextElapsedTime = elapsedTime < 0 ? 0 : elapsedTime;

    if (nextElapsedTime != m_ElapsedTimeSec)
    {
        m_ElapsedTimeSec = nextElapsedTime;
        m_CommandTimeoutPublisher.publish({m_ElapsedTimeSec});

        LOG_DEBUG("Command %d Elapsed Time: %d sec", m_CurrentCMD, m_ElapsedTimeSec);
    }
}

void GCSCommandHandler::reset()
{
    m_CurrentCMD = DATALINK_TELEMETRY_CMD_NONE;
    m_CommandActive = false;
    m_CommandStartTime = 0;

    LOG_DEBUG("Reset commander state");
}

void GCSCommandHandler::ack(bool success)
{
    reset();

    gcs_ack ack;
    ack.success = (uint8_t)success;

    datalink_message_t msg;
    datalink_pack_gcs_ack(&ack, &msg);

    m_CommandTimeoutPublisher.publish({m_ElapsedTimeSec});
    m_SerialPublisher.publish(msg);

    LOG_INFO("Sent %s ACK for command %d", success ? "success" : "failure", m_CurrentCMD);
}

void GCSCommandHandler::nack()
{
    m_CurrentCMD = m_RemoteCommandSeq;

    reset();

    datalink_message_t msg;
    datalink_pack_gcs_nack(&msg);

    m_SerialPublisher.publish(msg);

    LOG_INFO("Sent NACK for command %d", m_CurrentCMD);
}