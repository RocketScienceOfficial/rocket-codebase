#pragma once

#define GPS_RX_PIN 12
#define GPS_TX_PIN 34

struct GPSData
{
    double latitude;
    double longitude;
    float altitude;
};

void gps_init();
void gps_update();
const GPSData &gps_get_data();