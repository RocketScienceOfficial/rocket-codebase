#include "projection.h"
#include "physical_constants.h"
#include <lib/maths/math_constants.h>
#include <math.h>

void equirect_projection_init(equirect_projection_t *proj, const geo_position_t *ref)
{
    proj->ref = *ref;
    proj->ref_lat_rad = DEG_2_RAD(ref->lat);
    proj->ref_lon_rad = DEG_2_RAD(ref->lon);
    proj->ref_alt = ref->alt;
    proj->cos_lat = cos(proj->ref_lat_rad);
}

vec3_t equirect_project_to_ned(const equirect_projection_t *proj, const geo_position_t *pos)
{
    double lat_rad = DEG_2_RAD(pos->lat);
    double lon_rad = DEG_2_RAD(pos->lon);

    double n = EARTH_RADIUS * (lat_rad - proj->ref_lat_rad);
    double e = EARTH_RADIUS * proj->cos_lat * (lon_rad - proj->ref_lon_rad);
    double d = -(pos->alt - proj->ref_alt);

    return (vec3_t){
        .x = (float)n,
        .y = (float)e,
        .z = (float)d,
    };
}

geo_position_t equirect_unproject_from_ned(const equirect_projection_t *proj, const vec3_t *ned)
{
    double lat_rad = proj->ref_lat_rad + (double)ned->x / EARTH_RADIUS;
    double lon_rad = proj->ref_lon_rad + (double)ned->y / (EARTH_RADIUS * proj->cos_lat);
    double alt = proj->ref_alt - (double)ned->z;

    return (geo_position_t){
        .lat = RAD_2_DEG(lat_rad),
        .lon = RAD_2_DEG(lon_rad),
        .alt = alt,
    };
}