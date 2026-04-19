#include "com_uart.h"
#include "config.h"
#include "logger.h"
#include <pico/stdlib.h>

void CommunicationUART::init()
{
    uart_init(UART_INST, UART_BAUDRATE);

    gpio_init(UART_PIN_TX);
    gpio_set_dir(UART_PIN_TX, true);
    gpio_set_function(UART_PIN_TX, GPIO_FUNC_UART);

    gpio_init(UART_PIN_RX);
    gpio_set_dir(UART_PIN_RX, true);
    gpio_set_function(UART_PIN_RX, GPIO_FUNC_UART);
}

bool CommunicationUART::read(datalink_message_t *msg)
{
    while (uart_is_readable(UART_INST))
    {
        uint8_t byte;
        uart_read_blocking(UART_INST, &byte, 1);

        if (m_ReceiveLen >= sizeof(m_ReceiveBuffer))
        {
            m_ReceiveLen = 0;
        }

        m_ReceiveBuffer[m_ReceiveLen++] = byte;

        if (byte == 0x00)
        {
            int res = datalink_deserialize_message_serial(msg, m_ReceiveBuffer, m_ReceiveLen);

            m_ReceiveLen = 0;

            if (res == DATALINK_OK)
            {
                return true;
            }
            else
            {
                LOG_ERROR("Couldn't deserialize UART frame!");
            }
        }
    }

    return false;
}

void CommunicationUART::sendMessage(const datalink_message_t *msg)
{
    uint8_t buffer[512];
    int len = sizeof(buffer);

    if (datalink_serialize_message_serial(msg, buffer, &len) == DATALINK_OK)
    {
        uart_write_blocking(UART_INST, buffer, len);

        LOG_INFO("Sent %d bytes to UART!", len);
    }
    else
    {
        LOG_ERROR("Couldn't serialize UART frame!");
    }
}