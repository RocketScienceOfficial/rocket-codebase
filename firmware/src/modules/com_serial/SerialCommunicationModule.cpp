#include "SerialCommunicationModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/debug/sys_assert.h>
#include <hal/stdio_driver.h>

void SerialCommunicationModule::init()
{
}

void SerialCommunicationModule::run()
{
    flushStartup();

    drainTXBuffer();
    drainRXBuffer();
}

void SerialCommunicationModule::drainTXBuffer()
{
    for (uint8_t i = 0; i < SERIAL_MAX_TX_MSG_PER_TICK; i++)
    {
        if (m_Subscriber.poll())
        {
            send(m_Subscriber.get());
        }
        else
        {
            break;
        }
    }
}

void SerialCommunicationModule::drainRXBuffer()
{
    uint8_t byte = 0;
    uint8_t i = 0;

    while (hal_stdio_read_byte(&byte) && i < SERIAL_MAX_BYTES_PER_TICK)
    {
        if (m_CurrentReceiveBufferSize >= sizeof(m_ReceiveBuffer))
        {
            SYS_ASSERT_MSG(false, "Receive buffer overflow, dropping data");

            m_CurrentReceiveBufferSize = 0;
        }

        m_ReceiveBuffer[m_CurrentReceiveBufferSize++] = byte;

        if (byte == 0x00)
        {
            datalink_message_t msg;

            if (datalink_deserialize_message_serial(&msg, m_ReceiveBuffer, m_CurrentReceiveBufferSize) == DATALINK_OK)
            {
                m_Publisher.publish(msg);
            }
            else
            {
                LOG_ERROR("Failed to deserialize message from serial, dropping message");
            }

            m_CurrentReceiveBufferSize = 0;
        }

        i++;
    }
}

void SerialCommunicationModule::send(const datalink_message_t &message)
{
    int len = sizeof(m_SendBuffer);

    if (datalink_serialize_message_serial(&message, m_SendBuffer, &len) == DATALINK_OK)
    {
        hal_stdio_send_buffer(m_SendBuffer, len);
    }
    else
    {
        SYS_ASSERT_MSG(false, "Failed to serialize message for Serial transmission, dropping message");
    }
}

void SerialCommunicationModule::flushStartup()
{
    if (!m_SerialStartFlushed)
    {
        uint8_t i = 0;
        uint8_t byte = 0;

        while (hal_stdio_read_byte(&byte) && i++ < SERIAL_STARTUP_FLUSH_MAX_BYTES)
        {
            continue;
        }

        SYS_ASSERT_MSG(i < SERIAL_STARTUP_FLUSH_MAX_BYTES, "Serial startup flush exceeded max bytes, possible continuous data stream on Serial");

        m_SerialStartFlushed = true;
    }
}