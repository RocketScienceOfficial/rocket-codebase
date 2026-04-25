#include "SimpleGPSModule.h"
#include "modules/common/ModuleLogger.h"
#include <board_config.h>
#include <hal/uart_driver.h>
#include <lib/debug/sys_assert.h>

void SimpleGPSModule::init()
{
}

void SimpleGPSModule::run()
{
    if (hal_uart_fifo_available(CFG_UART))
    {
        size_t bytes_read = hal_uart_read_fifo(CFG_UART, m_ReceiveFIFOBuffer, sizeof(m_ReceiveFIFOBuffer));

        for (size_t i = 0; i < bytes_read; i++)
        {
            const uint8_t &byte = m_ReceiveFIFOBuffer[i];

            if (m_CurrentSentenceLength >= sizeof(m_CurrentSentence) - 1)
            {
                SYS_ASSERT_MSG(false, "Receive buffer overflow, dropping data");

                m_CurrentSentenceLength = 0;
            }

            if (byte == '$')
            {
                m_CurrentSentenceLength = 0;
            }

            m_CurrentSentence[m_CurrentSentenceLength++] = byte;

            if (byte == '\n')
            {
                m_CurrentSentence[m_CurrentSentenceLength++] = '\0';

                parseSentence();
            }
        }
    }
}

static float _get_lat_sign(char ns)
{
    return ns == 'S' ? -1 : 1;
}

static float _get_lon_sign(char ew)
{
    return ew == 'W' ? -1 : 1;
}

void SimpleGPSModule::parseSentence()
{
    if (!nmea_check_sentence(m_CurrentSentence))
    {
        LOG_ERROR("Invalid NMEA sentence: %s", m_CurrentSentence);
        return;
    }

    nmea_sentence_id_t s = nmea_get_sentence_id(m_CurrentSentence);

    if (s == NMEA_SENTENCE_GGA)
    {
        nmea_sentence_gga_t frame;
        bool res = nmea_parse_gga(m_CurrentSentence, &frame);

        if (!res)
        {
            LOG_ERROR("Failed to parse GGA sentence: %s", m_CurrentSentence);
            return;
        }

        m_Publisher.publish({
            .lat = frame.lat * _get_lat_sign(frame.NS),
            .lon = frame.lon * _get_lon_sign(frame.EW),
            .alt = frame.alt,
        });
    }
}