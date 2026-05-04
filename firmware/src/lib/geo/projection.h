#ifndef _PROJECTION_H
#define _PROJECTION_H

#include <lib/maths/vector.h>
#include <lib/geo/wgs84.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Structure for holding equirectangular projection parameters
 */
typedef struct
{
    double ref_lat_rad;
    double ref_lon_rad;
    double ref_alt;
    double cos_lat;
} equirect_projection_t;

/**
 * @brief Initialize equirectangular projection with reference position
 * 
 * @param proj Pointer to projection structure to initialize
 * @param ref Reference position for the projection
 */
void equirect_projection_init(equirect_projection_t *proj, const geo_position_t *ref);

/**
 * @brief Project geo position to NED frame using equirectangular projection
 * 
 * @param proj Pointer to initialized projection structure
 * @param pos Geo position to project
 * @return Projected position in NED frame
 */
vec3_t equirect_project_to_ned(const equirect_projection_t *proj, const geo_position_t *pos);

/**
 * @brief Unproject NED position to geo position using equirectangular projection
 * 
 * @param proj Pointer to initialized projection structure
 * @param ned NED position to unproject
 * @return Unprojected geo position
 */
geo_position_t equirect_unproject_from_ned(const equirect_projection_t *proj, const vec3_t *ned);

#ifdef __cplusplus
}
#endif

#endif
