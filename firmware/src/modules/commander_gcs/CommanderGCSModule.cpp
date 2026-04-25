#include "CommanderGCSModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>
#include <lib/debug/sys_assert.h>

#define CMD_TIMEOUT_MS 10000
#define RESPONSE_SEND_DELAY_MS 100
#define TMP_RX_RESET_TIME 5000

void CommanderGCSModule::init()
{
}

void CommanderGCSModule::run()
{
    m_GPSSubscriber.pollLatest();

    while (m_SerialSubscriber.poll())
    {
        processSerialMessage(m_SerialSubscriber.get());
    }

    while (m_RadioSubscriber.poll())
    {
        processRadioMessage(m_RadioSubscriber.get());
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

    if (m_ResponseStartTime != 0 && osal_systime_get_ms() - m_ResponseStartTime > RESPONSE_SEND_DELAY_MS)
    {
        sendTelemetryResponse();

        m_ResponseStartTime = 0;
    }

    if (osal_systime_get_ms() - m_RadioTmpRXStartTime > TMP_RX_RESET_TIME)
    {
        m_RadioTmpRX = 0;
        m_PacketsLost = 0;
        m_RadioTmpRXStartTime = osal_systime_get_ms();
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

void CommanderGCSModule::processRadioMessage(const PubSub::Topics::LoRaRXData &data)
{
    m_RadioRX++;
    m_RadioTmpRX++;

    updateRadioState();

    if (data.sequence != m_RXSequence)
    {
        m_PacketsLost += data.sequence > m_RXSequence ? data.sequence - m_RXSequence : 256 + data.sequence - m_RXSequence;
        m_RXSequence = data.sequence + 1;
    }
    else if (m_RXSequence == 255)
    {
        m_RXSequence = 0;
    }
    else
    {
        m_RXSequence++;
    }

    if (data.msg.msg_id != DATALINK_MESSAGE_ID_TELEMETRY_DATA_OBC)
    {
        LOG_ERROR("Invalid message id! Expected %d, got %d", DATALINK_MESSAGE_ID_TELEMETRY_DATA_OBC, data.msg.msg_id);

        return;
    }

    telemetry_data_obc frame;
    int status = datalink_unpack_telemetry_data_obc(&frame, &data.msg);

    SYS_ASSERT(status == DATALINK_OK);

    telemetry_data_gcs newPayload;
    newPayload.qw = frame.qw;
    newPayload.qx = frame.qx;
    newPayload.qy = frame.qy;
    newPayload.qz = frame.qz;
    newPayload.velocity_kmh = frame.velocity_kmh;
    newPayload.batteryVoltage100 = frame.batteryVoltage100;
    newPayload.batteryPercentage = frame.batteryPercentage;
    newPayload.lat = frame.lat;
    newPayload.lon = frame.lon;
    newPayload.alt = frame.alt;
    newPayload.gcsLat = (int)(m_GPSSubscriber.get().lat * 10000000);
    newPayload.gcsLon = (int)(m_GPSSubscriber.get().lon * 10000000);
    newPayload.gpsData = frame.gpsData;
    newPayload.state = frame.state;
    newPayload.stateFlags = frame.stateFlags;
    newPayload.signalStrengthNeg = (uint8_t)-data.rssi;
    newPayload.packetLossPercentage = (uint8_t)((float)m_PacketsLost / (m_RadioTmpRX + m_PacketsLost) * 100);
    newPayload.packetsReceived = (uint16_t)m_RadioRX;
    newPayload.packetsTransmitted = (uint16_t)m_RadioTX;

    datalink_message_t msg;
    datalink_pack_telemetry_data_gcs(&newPayload, &msg);
    m_SerialPublisher.publish(msg);

    if (frame.sendResponse == 1)
    {
        m_ResponseStartTime = osal_systime_get_ms();
    }

    handleNewRadioSequence(frame.cmd_seq, frame.cmd_last_status);
}

void CommanderGCSModule::handleNewRadioSequence(uint8_t seq, uint8_t status)
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

void CommanderGCSModule::sendTelemetryResponse()
{
    telemetry_response payload;
    payload.cmd = m_CurrentCMD;
    payload.cmd_seq = m_CurrentCommandSeq;

    datalink_message_t msg;
    datalink_pack_telemetry_response(&payload, &msg);

    m_RadioPublisher.publish({
        .msg = msg,
        .sequence = m_TXSequence,
    });

    m_TXSequence = m_TXSequence == 255 ? 0 : m_TXSequence + 1;
    m_RadioTX++;

    updateRadioState();

    LOG_INFO("Sent telemetry response for command %d (seq %d)", m_CurrentCMD, m_CurrentCommandSeq);
}

void CommanderGCSModule::updateRadioState()
{
    m_RadioStatePublisher.publish({
        .rx = m_RadioRX,
        .tx = m_RadioTX,
    });
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

    LOG_DEBUG("Reset commander state");
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