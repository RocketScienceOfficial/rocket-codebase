#include "CommanderModule.h"
#include "modules/common/ModuleLogger.h"
#include <osal/systime.h>
#include <lib/debug/obc_assert.h>

#define RADIO_COMMAND_TIMEOUT 10000

#define SERIAL_SRC_ID 0
#define RADIO_SRC_ID 1

void CommanderModule::init()
{
}

void CommanderModule::run()
{
    while (m_SerialSubscriber.poll())
    {
        processSerialMessage(m_SerialSubscriber.get());
    }

    while (m_RadioSubscriber.poll())
    {
        processRadioMessage(m_RadioSubscriber.get());
    }

    while (m_DatabaseSubscriber.poll())
    {
        const datalink_message_t &msg = m_DatabaseSubscriber.get();

        m_SerialPublisher.publish(msg);
    }

    while (m_TelemetrySubscriber.poll())
    {
        const telemetry_data_obc &tlmData = m_TelemetrySubscriber.get();

        datalink_message_t msg;
        datalink_pack_telemetry_data_obc(&tlmData, &msg);

        m_RadioPublisher.publish(msg);
    }

    handleRPCs();
}

void CommanderModule::handleRPCs()
{
    if (m_RadioCommandStatus == CommanderStatus::PENDING && osal_systime_get_ms() - m_LastRadioCommandTime > RADIO_COMMAND_TIMEOUT)
    {
        OBC_ERROR("Radio command timed out");

        setRadioRPCStatus(false);
    }

    if (m_RPC_ARM.finished() && m_RPC_ARM.getResponseSource() == RADIO_SRC_ID)
    {
        setRadioRPCStatus(m_RPC_ARM.isSuccess());
    }
    if (m_RPC_Voltage.finished() && m_RPC_Voltage.getResponseSource() == RADIO_SRC_ID)
    {
        setRadioRPCStatus(m_RPC_Voltage.isSuccess());
    }
    if (m_RPC_IGN.finished())
    {
        if (m_RPC_IGN.getResponseSource() == RADIO_SRC_ID)
        {
            setRadioRPCStatus(m_RPC_IGN.isSuccess());
        }
        else if (m_RPC_IGN.getResponseSource() == SERIAL_SRC_ID)
        {
            datalink_message_t msg;
            datalink_pack_ign_response_test(&msg);

            m_SerialPublisher.publish(msg);
        }
    }
}

void CommanderModule::processSerialMessage(const datalink_message_t &msg)
{
    switch (msg.msg_id)
    {
    case DATALINK_MESSAGE_ID_IGN_REQUEST_TEST:
    {
        ign_request_test req;
        int status = datalink_unpack_ign_request_test(&req, &msg);

        OBC_ASSERT(status == DATALINK_OK);

        m_RPC_IGN.call({.channel = req.ignNum}, SERIAL_SRC_ID);

        break;
    }
    default:
    {
        m_DatabasePublisher.publish(msg);
        break;
    }
    }
}

void CommanderModule::processRadioMessage(const datalink_message_t &msg)
{
    switch (msg.msg_id)
    {
    case DATALINK_MESSAGE_ID_TELEMETRY_RESPONSE:
    {
        telemetry_response tlmData;
        int status = datalink_unpack_telemetry_response(&tlmData, &msg);

        OBC_ASSERT(status == DATALINK_OK);

        m_TelemetryResponsePublisher.publish(tlmData);

        uint8_t nextPredictedSeq = (m_RadioCommandSeq + 1) % 256;

        if (tlmData.cmd == DATALINK_TELEMETRY_CMD_NONE)
        {
            OBC_DEBUG("Received telemetry response with CMD_NONE");
            return;
        }
        if (tlmData.cmd_seq != nextPredictedSeq)
        {
            OBC_DEBUG("Received telemetry response with invalid cmd_seq %d (current %d)", tlmData.cmd_seq, m_RadioCommandSeq);
            return;
        }
        if (m_RadioCommandStatus == CommanderStatus::PENDING)
        {
            OBC_DEBUG("Received telemetry response while pending - ignoring command");
            return;
        }

        executeRadioCommand(tlmData.cmd);

        m_RadioCommandSeq = nextPredictedSeq;
        m_RadioCommandStatus = CommanderStatus::PENDING;
        m_LastRadioCommandTime = osal_systime_get_ms();

        updateState();

        break;
    }
    case DATALINK_MESSAGE_ID_RADIO_MODULE_TX_DONE:
    {
        m_RadioACKPublisher.publish({0});

        break;
    }
    default:
    {
        OBC_ERROR("Received message with unknown ID %d on radio interface", msg.msg_id);
        return;
    }
    }
}

void CommanderModule::executeRadioCommand(uint8_t cmd)
{
    using namespace PubSub::Helpers;

    OBC_DEBUG("Executing radio command %d", cmd);

    switch (cmd)
    {
    case DATALINK_TELEMETRY_CMD_ARM:
    {
        m_RPC_ARM.call({.arm = true}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_DISARM:
    {
        m_RPC_ARM.call({.arm = false}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_3V3_ENABLED:
    {
        m_RPC_Voltage.call({.pin = VoltagePinsFlags::VOLTAGE_PIN_3V3, .enabled = true}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_3V3_DISABLED:
    {
        m_RPC_Voltage.call({.pin = VoltagePinsFlags::VOLTAGE_PIN_3V3, .enabled = false}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_5V_ENABLED:
    {
        m_RPC_Voltage.call({.pin = VoltagePinsFlags::VOLTAGE_PIN_5V, .enabled = true}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_5V_DISABLED:
    {
        m_RPC_Voltage.call({.pin = VoltagePinsFlags::VOLTAGE_PIN_5V, .enabled = false}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_VBAT_ENABLED:
    {
        m_RPC_Voltage.call({.pin = VoltagePinsFlags::VOLTAGE_PIN_VBAT, .enabled = true}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_VBAT_DISABLED:
    {
        m_RPC_Voltage.call({.pin = VoltagePinsFlags::VOLTAGE_PIN_VBAT, .enabled = false}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_IGN_1_REQ_FIRE:
    {
        m_RPC_IGN.call({.channel = 1}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_IGN_2_REQ_FIRE:
    {
        m_RPC_IGN.call({.channel = 2}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_IGN_3_REQ_FIRE:
    {
        m_RPC_IGN.call({.channel = 3}, RADIO_SRC_ID);
        break;
    }
    case DATALINK_TELEMETRY_CMD_IGN_4_REQ_FIRE:
    {
        m_RPC_IGN.call({.channel = 4}, RADIO_SRC_ID);
        break;
    }
    default:
    {
        OBC_ERROR("Received unknown radio command %d", cmd);
        break;
    }
    }
}

void CommanderModule::setRadioRPCStatus(bool success)
{
    if (m_RadioCommandStatus != CommanderStatus::PENDING)
    {
        OBC_ERROR("Attempted to set radio RPC status while not pending");

        return;
    }

    m_RadioCommandStatus = success ? CommanderStatus::SUCCESS : CommanderStatus::FAILURE;

    updateState();
}

void CommanderModule::updateState()
{
    m_CommanderRadioRPCStatePublisher.publish({.seq = m_RadioCommandSeq, .status = m_RadioCommandStatus});

    OBC_DEBUG("Updated commander state: seq=%d, status=%d", m_RadioCommandSeq, (uint8_t)m_RadioCommandStatus);
}