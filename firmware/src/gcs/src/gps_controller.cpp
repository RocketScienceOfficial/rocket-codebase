#include "gps_controller.h"
#include "logger.h"
#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <MicroNMEA.h>

static SFE_UBLOX_GNSS g_gnss;
static char g_buffer[100];
static MicroNMEA g_nmea(g_buffer, sizeof(g_buffer));
static GPSData g_data;

void gps_init()
{
    Serial1.begin(9600, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);

    if (!g_gnss.begin(Serial1))
    {
        SERIAL_DEBUG_PRINTF("GPS failed to start!\n");

        while (1)
            ;
    }

    g_gnss.setUART1Output(COM_TYPE_NMEA);
    g_gnss.saveConfiguration();

    SERIAL_DEBUG_PRINTF("GPS started!\n");
}

void gps_update()
{
    if (g_gnss.checkUblox())
    {
        if (g_nmea.isValid())
        {
            long lat = g_nmea.getLatitude();
            long lon = g_nmea.getLongitude();
            long alt = 0;

            g_data.latitude = lat / 1000000.0;
            g_data.longitude = lon / 1000000.0;

            g_nmea.getAltitude(alt);

            g_data.altitude = alt / 1000.0f;

            SERIAL_DEBUG_PRINTF("Updated GPS!\n");
        }
    }
}

const GPSData &gps_get_data()
{
    return g_data;
}

void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
    g_nmea.process(incoming);
}