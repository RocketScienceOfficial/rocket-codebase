#include "RadioCommunicationModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <lib/debug/sys_assert.h>
#include <hal/uart_driver.h>

void RadioCommunicationModule::init()
{
}

void RadioCommunicationModule::run()
{
    flushStartup();

    drainTXBuffer();
    drainRXBuffer();
}

void RadioCommunicationModule::drainTXBuffer()
{
    if (!hal_uart_is_writable(CFG_UART))
    {
        return;
    }

    uint8_t i = 0;

    while (m_Subscriber.poll() && i < UART_MAX_TX_MSG_PER_TICK)
    {
        addToSendQueue(m_Subscriber.get());

        i++;
    }

    if (i > 0 && m_CurrentSendBufferSize > 0)
    {
        hal_uart_write(CFG_UART, m_SendBuffer, m_CurrentSendBufferSize);

        LOG_DEBUG("Sent %d messages over UART (%lu bytes)", i, (unsigned long)m_CurrentSendBufferSize);

        m_CurrentSendBufferSize = 0;
    }
}

void RadioCommunicationModule::drainRXBuffer()
{
    if (!hal_uart_fifo_available(CFG_UART))
    {
        return;
    }

    size_t bytes_read = hal_uart_read_fifo(CFG_UART, m_ReceiveFIFOBuffer, sizeof(m_ReceiveFIFOBuffer));

    for (size_t i = 0; i < bytes_read; i++)
    {
        const uint8_t &byte = m_ReceiveFIFOBuffer[i];

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
                LOG_ERROR("Failed to deserialize message from UART, dropping message");
            }

            m_CurrentReceiveBufferSize = 0;
        }
    }
}

void RadioCommunicationModule::addToSendQueue(const datalink_message_t &message)
{
    int len = sizeof(m_SendBuffer) - m_CurrentSendBufferSize;

    if (datalink_serialize_message_serial(&message, m_SendBuffer + m_CurrentSendBufferSize, &len) == DATALINK_OK)
    {
        m_CurrentSendBufferSize += len;
    }
    else
    {
        SYS_ASSERT_MSG(false, "Failed to serialize message for UART transmission, dropping message");
    }
}

void RadioCommunicationModule::flushStartup()
{
    if (!m_UARTStartFlushed)
    {
        uint8_t i = 0;
        uint8_t dummyBuffer[UART_MAX_BYTES_PER_TICK];

        while (hal_uart_fifo_available(CFG_UART) && i++ < UART_STARTUP_FLUSH_MAX_TRIES)
        {
            hal_uart_read_fifo(CFG_UART, dummyBuffer, sizeof(dummyBuffer));
        }

        SYS_ASSERT_MSG(i < UART_STARTUP_FLUSH_MAX_TRIES, "UART startup flush exceeded max tries, possible continuous data stream on UART");

        m_UARTStartFlushed = true;
    }
}