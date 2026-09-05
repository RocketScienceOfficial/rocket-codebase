#include "GPSModule.h"
#include <lib/drivers_utils/spi_utils.h>
#include <cstring>

#define MAX_READ_BYTES 32

void GPSModule::init()
{
    configureSPI();
}

void GPSModule::run()
{
    uint8_t i = 0;
    uint8_t byte;

    spi_utils_cs_select(m_CS);

    while (i++ < MAX_READ_BYTES)
    {
        hal_spi_transfer(m_SPI, NULL, &byte, 1);
        ubx_parser_status_t status = ubx_process_byte(&m_Parser, byte);

        if (status == UBX_PARSER_STATUS_FINISHED)
        {
            m_CurrentFrame.pos.lat = m_Parser.current_frame.lat * 1e-7;
            m_CurrentFrame.pos.lon = m_Parser.current_frame.lon * 1e-7;
            m_CurrentFrame.pos.alt = m_Parser.current_frame.height * 1e-3;
            m_CurrentFrame.vel.x = m_Parser.current_frame.velN * 1e-3;
            m_CurrentFrame.vel.y = m_Parser.current_frame.velE * 1e-3;
            m_CurrentFrame.vel.z = m_Parser.current_frame.velD * 1e-3;
            m_CurrentFrame.stddev_horizontal = m_Parser.current_frame.hAcc * 1e-3;
            m_CurrentFrame.stddev_vertical = m_Parser.current_frame.vAcc * 1e-3;
            m_CurrentFrame.stddev_speed = m_Parser.current_frame.sAcc * 1e-3;
            m_CurrentFrame.gpsFix = m_Parser.current_frame.fixType >= 2;
            m_CurrentFrame.gpsIs3dFix = m_Parser.current_frame.fixType >= 3;

            m_Publisher.publish(m_CurrentFrame);

            break;
        }
        else if (status == UBX_PARSER_STATUS_PARSING)
        {
            continue;
        }
        else if (status == UBX_PARSER_STATUS_UNAVAILABLE)
        {
            break;
        }
    }

    spi_utils_cs_deselect(m_CS);
}

void GPSModule::configureSPI()
{
    ubx_cfg_builder_t cfgBuilder = {};

    ubx_set_nmea_enabled_spi(&cfgBuilder, false);
    ubx_set_pvt_enabled_spi(&cfgBuilder, true);
    ubx_set_nav_rate(&cfgBuilder, 1000 / 25);
    ubx_set_airborne_dynamic_model(&cfgBuilder);

    uint8_t buffer[256];
    size_t len = ubx_valset_apply(&cfgBuilder, buffer, sizeof(buffer));

    spi_utils_cs_init(m_CS);
    spi_utils_cs_select(m_CS);
    hal_spi_transfer(m_SPI, buffer, NULL, len);
    spi_utils_cs_deselect(m_CS);
}