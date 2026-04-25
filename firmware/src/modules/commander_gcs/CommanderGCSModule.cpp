#include "CommanderGCSModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>
#include <lib/debug/sys_assert.h>

#define CMD_TIMEOUT_MS 10000

void CommanderGCSModule::init()
{
}

void CommanderGCSModule::run()
{
    while (m_SerialSubscriber.poll())
    {
        processSerialMessage(m_SerialSubscriber.get());
    }

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

void CommanderGCSModule::processSerialMessage(const datalink_message_t &msg)
{
    switch (msg.msg_id)
    {
    case DATALINK_MESSAGE_ID_TELEMETRY_GCS_CMD:
    {
        telemetry_gcs_cmd payload;
        int status = datalink_unpack_telemetry_gcs_cmd(&payload, &msg);

        SYS_ASSERT(status == DATALINK_OK);

        set(payload.cmd);

        break;
    }
    }
}

void CommanderGCSModule::processNewRadioData(uint8_t seq, uint8_t status)
{
    m_RemoteCommandSeq = seq;

    LOG_INFO("Remote Seq: %d Status: %d Current: %d\n", m_RemoteCommandSeq, status, m_CurrentCommandSeq);

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

void CommanderGCSModule::handleCommandElapsedTime()
{
    uint32_t diff = osal_systime_get_ms() - m_CommandStartTime;
    int elapsedTime = (CMD_TIMEOUT_MS - (int)diff) / 1000;
    int nextElapsedTime = elapsedTime < 0 ? 0 : elapsedTime;

    if (nextElapsedTime != m_ElapsedTimeSec)
    {
        m_ElapsedTimeSec = nextElapsedTime;
        m_CommandTimeoutPublisher.publish({m_ElapsedTimeSec});

        LOG_DEBUG("Command %d Elapsed Time: %d sec\n", m_CurrentCMD, m_ElapsedTimeSec);
    }
}

void CommanderGCSModule::set(uint8_t cmd)
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

void CommanderGCSModule::reset()
{
    m_CurrentCMD = DATALINK_TELEMETRY_CMD_NONE;
    m_CommandActive = false;
    m_CommandStartTime = 0;
}

void CommanderGCSModule::ack(bool success)
{
    reset();

    gcs_ack ack;
    ack.success = (uint8_t)success;

    datalink_message_t msg;
    datalink_pack_gcs_ack(&ack, &msg);

    m_SerialPublisher.publish(msg);

    LOG_INFO("Sent %s ACK for command %d", success ? "success" : "failure", m_CurrentCMD);
}

void CommanderGCSModule::nack()
{
    m_CurrentCMD = m_RemoteCommandSeq;

    reset();

    datalink_message_t msg;
    datalink_pack_gcs_nack(&msg);

    m_SerialPublisher.publish(msg);

    LOG_INFO("Sent NACK for command %d", m_CurrentCMD);
}