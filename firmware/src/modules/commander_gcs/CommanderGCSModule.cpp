#include "CommanderGCSModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>
#include <lib/debug/sys_assert.h>

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

    m_CommandHandler.update();

    checkPacketLossResetTimeout();
    checkTelemetryResponseTimeout();
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

        m_CommandHandler.set(payload.cmd);

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

    LOG_INFO("Received radio message (id: %d) with seq %d (rssi: %d)", data.msg.msg_id, data.sequence, data.rssi);

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

    m_CommandHandler.onNewSequence(frame.cmd_seq, frame.cmd_last_status);
}

void CommanderGCSModule::checkPacketLossResetTimeout()
{
    if (osal_systime_get_ms() - m_RadioTmpRXStartTime > TMP_RX_RESET_TIME)
    {
        LOG_DEBUG("Packet loss stats reset (%d packets lost in last %d seconds)", m_PacketsLost, TMP_RX_RESET_TIME / 1000);

        m_RadioTmpRX = 0;
        m_PacketsLost = 0;
        m_RadioTmpRXStartTime = osal_systime_get_ms();
    }
}

void CommanderGCSModule::checkTelemetryResponseTimeout()
{
    if (m_ResponseStartTime != 0 && osal_systime_get_ms() - m_ResponseStartTime > RESPONSE_SEND_DELAY_MS)
    {
        sendTelemetryResponse();

        m_ResponseStartTime = 0;
    }
}

void CommanderGCSModule::sendTelemetryResponse()
{
    telemetry_response payload;
    payload.cmd = m_CommandHandler.getCurrentCommand();
    payload.cmd_seq = m_CommandHandler.getCurrentSequence();

    datalink_message_t msg;
    datalink_pack_telemetry_response(&payload, &msg);

    m_RadioPublisher.publish({
        .msg = msg,
        .sequence = m_TXSequence,
    });

    m_TXSequence = m_TXSequence == 255 ? 0 : m_TXSequence + 1;
    m_RadioTX++;

    updateRadioState();

    LOG_INFO("Sent telemetry response for command %d (seq %d)", payload.cmd, payload.cmd_seq);
}

void CommanderGCSModule::updateRadioState()
{
    m_RadioStatePublisher.publish({
        .rx = m_RadioRX,
        .tx = m_RadioTX,
    });
}