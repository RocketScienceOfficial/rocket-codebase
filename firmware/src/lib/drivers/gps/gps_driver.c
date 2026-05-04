#include "gps_driver.h"
#include "ubx.h"
#include "spi_utils.h"
#include <lib/geo/physical_constants.h>
#include <lib/debug/sys_assert.h>
#include <hal/spi_driver.h>
#include <string.h>

#define GPS_MAX_READ_BYTES 32

static void _configure_spi(gps_device_t *device)
{
    SYS_ASSERT(device != NULL);

    ubx_set_nmea_enabled_spi(false);
    ubx_set_pvt_enabled_spi(true);
    ubx_set_nav_rate(1000 / 25);
    ubx_set_airborne_dynamic_model();

    uint8_t buffer[256];
    size_t len = ubx_valset_apply(buffer, sizeof(buffer));

    spi_utils_cs_select(device->cs);
    hal_spi_transfer(device->spi, buffer, NULL, len);
    spi_utils_cs_deselect(device->cs);
}

void gps_init_spi(gps_device_t *device, uint8_t spi, uint8_t cs)
{
    SYS_ASSERT(device != NULL);

    device->spi = spi;
    device->cs = cs;

    spi_utils_cs_init(cs);

    memset(&device->parser, 0, sizeof(device->parser));
    memset(&device->data, 0, sizeof(device->data));

    _configure_spi(device);
}

bool gps_read_spi(gps_device_t *device)
{
    SYS_ASSERT(device != NULL);
    
    bool found = false;
    uint8_t i = 0;
    uint8_t byte;

    spi_utils_cs_select(device->cs);

    while (i++ < GPS_MAX_READ_BYTES)
    {
        hal_spi_transfer(device->spi, NULL, &byte, 1);
        ubx_parser_status_t status = ubx_process_byte(&device->parser, byte);

        if (status == UBX_PARSER_STATUS_FINISHED)
        {
            device->data.position.lat = device->parser.current_frame.lat * 1e-7;
            device->data.position.lon = device->parser.current_frame.lon * 1e-7;
            device->data.position.alt = device->parser.current_frame.height * 1e-3;
            device->data.velocity.x = device->parser.current_frame.velN * 1e-3;
            device->data.velocity.y = device->parser.current_frame.velE * 1e-3;
            device->data.velocity.z = device->parser.current_frame.velD * 1e-3;
            device->data.fix = device->parser.current_frame.fixType >= 2;
            device->data.is3dFix = device->parser.current_frame.fixType >= 3;
            device->data.numSV = device->parser.current_frame.numSV;

            found = true;
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

    spi_utils_cs_deselect(device->cs);

    return found;
}