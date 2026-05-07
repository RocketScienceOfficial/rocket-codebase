#ifndef _GEO_MAG_H
#define _GEO_MAG_H

#include "wgs84.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get magnetic declination at location (altitude is ignored)
 *
 * @param pos Coordinates
 * @return Declination in radians
 */
float geo_mag_get_declination(const geo_position_t* pos);

/**
 * @brief Get magnetic inclination at location (altitude is ignored)
 *
 * @param pos Coordinates
 * @return Inclination in radians
 */
float geo_mag_get_inclination(const geo_position_t* pos);

/**
 * @brief Get magnetic strength at location (altitude is ignored)
 *
 * @param pos Coordinates
 * @return Strength in Gauss
 */
float geo_mag_get_strength(const geo_position_t* pos);

/**
 * @brief Get magnetic field vector in NED frame at location (altitude is ignored). The returned vector is not normalized and its magnitude corresponds to the magnetic strength at the location.
 * 
 * @param pos Coordinates
 * @return Magnetic field vector in NED frame (x = north, y = east, z = down)
 */
vec3_t geo_mag_field_vector(const geo_position_t* pos);

#ifdef __cplusplus
}
#endif

#endif