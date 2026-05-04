#ifndef _WGS84_H
#define _WGS84_H

#include <lib/maths/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure for holding WGS 84 geo coordinates
 */
typedef struct
{
    double lat; /** Latitude in degrees */
    double lon; /** Longitude in degrees */
    double alt; /** Altitude in meters */
} geo_position_t;

/**
 * @brief Get XYZ position in NED frame from geo positions
 *
 * @param basePos Position of reference point
 * @param pos Position of current point
 * @return XYZ position
 */
vec3_prec_t wgs84_geo_to_ned(geo_position_t basePos, geo_position_t pos);

/**
 * @brief Get geo position from NED frame
 *
 * @param basePos Position of reference point
 * @param pos NED frame position
 * @return Geo position
 */
geo_position_t wgs84_ned_to_geo(geo_position_t basePos, vec3_prec_t pos);

#ifdef __cplusplus
}
#endif

#endif