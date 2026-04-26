#include "CommanderRMModule.h"
#include "modules/common/ModuleLogger.h"

void CommanderRMModule::init()
{
}

void CommanderRMModule::run()
{
    if (m_UARTSubscriber.poll())
    {
        processUARTMessage(m_UARTSubscriber.get());
    }

    if (m_RadioSubscriber.poll())
    {
        processRadioMessage(m_RadioSubscriber.get());
    }

    if (m_AckSubscriber.poll())
    {
        processRadioAck();
    }
}

void CommanderRMModule::processUARTMessage(const datalink_message_t &msg)
{
    LOG_DEBUG("Received message from UART! (ID: %d, Payload Length: %d)", msg.msg_id, msg.len);

    if (msg.msg_id == DATALINK_MESSAGE_ID_TELEMETRY_DATA_OBC)
    {
        m_RadioPublisher.publish({
            .msg = msg,
            .sequence = m_Sequence,
        });

        m_Sequence = m_Sequence == 255 ? 0 : m_Sequence + 1;
    }
    else
    {
        LOG_ERROR("Unknown message ID received from UART: %d", msg.msg_id);
    }
}

void CommanderRMModule::processRadioMessage(const PubSub::Topics::LoRaRXData &data)
{
    LOG_DEBUG("Received message from radio! (ID: %d, Payload Length: %d, RSSI: %d, Sequence: %d)", data.msg.msg_id, data.msg.len, data.rssi, data.sequence);

    if (data.msg.msg_id == DATALINK_MESSAGE_ID_TELEMETRY_RESPONSE)
    {
        m_UARTPublisher.publish(data.msg);
    }
    else
    {
        LOG_ERROR("Unknown message ID received from radio: %d", data.msg.msg_id);
    }
}

void CommanderRMModule::processRadioAck()
{
    LOG_DEBUG("Radio ack received, sending UART ack");
    
    datalink_message_t msg;
    datalink_pack_radio_module_tx_done(&msg);
    m_UARTPublisher.publish(msg);
}