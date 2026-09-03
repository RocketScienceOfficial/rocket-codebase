#ifndef _GPS_DRIVER_H
#define _GPS_DRIVER_H

#include "ubx.h"
#include "bus_utils.h"
#include <lib/geo/wgs84.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief GPS cache data structure
 */
typedef struct
{
    geo_position_t position;
    vec3_t velocity_ned;
    float stddev_horizontal;
    float stddev_vertical;
    float stddev_speed;
    bool fix;
    bool is3dFix;
    uint8_t numSV;
} gps_data_t;

/**
 * @brief GPS device
 */
typedef struct gps_device
{
    uint8_t spi;
    uint8_t cs;
    ubx_parser_t parser;
    gps_data_t data;
} gps_device_t;

/**
 * @brief Initializes the GPS module
 *
 * @param device The GPS device
 * @param spi The SPI instance to use
 * @param cs The CS pin to use
 */
void gps_init_spi(gps_device_t *device, uint8_t spi, uint8_t cs);

/**
 * @brief Reads data from the GPS module (via SPI)
 *
 * @param device The GPS device
 * @return True if next read is available
 */
bool gps_read_spi(gps_device_t *device);

#ifdef __cplusplus
}
#endif

#endif