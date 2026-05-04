#include "wgs84.h"
#include "physical_constants.h"
#include <lib/maths/math_constants.h>
#include <math.h>

static void _geo_pos_to_rad(geo_position_t *pos)
{
    pos->lat = DEG_2_RAD(pos->lat);
    pos->lon = DEG_2_RAD(pos->lon);
}

static void _geo_pos_to_deg(geo_position_t *pos)
{
    pos->lat = RAD_2_DEG(pos->lat);
    pos->lon = RAD_2_DEG(pos->lon);
}

static vec3_prec_t _geo_to_ecef(geo_position_t pos)
{
    double sin_lat = sin(pos.lat);
    double cos_lat = cos(pos.lat);

    double n = EARTH_SEMI_MAJOR_AXIS / sqrt(1.0 - EARTH_ECCENTRICITY_SQUARED * sin_lat * sin_lat);

    return (vec3_prec_t){
        .x = (n + pos.alt) * cos_lat * cos(pos.lon),
        .y = (n + pos.alt) * cos_lat * sin(pos.lon),
        .z = ((1.0 - EARTH_ECCENTRICITY_SQUARED) * n + pos.alt) * sin_lat,
    };
}

static geo_position_t _ecef_to_geo(vec3_prec_t pos)
{
    double s = sqrt(pos.x * pos.x + pos.y * pos.y);

    double beta = atan(pos.z / ((1.0 - EARTH_FLATTENING) * s));
    double sin_beta = sin(beta);
    double cos_beta = cos(beta);
    double miu = atan(
        (pos.z + EARTH_ECCENTRICITY_SQUARED * (1.0 - EARTH_FLATTENING) / (1.0 - EARTH_ECCENTRICITY_SQUARED) * EARTH_SEMI_MAJOR_AXIS * sin_beta * sin_beta * sin_beta) / (s - EARTH_ECCENTRICITY_SQUARED * EARTH_SEMI_MAJOR_AXIS * cos_beta * cos_beta * cos_beta));
    double err = 1e10;
    int iter = 0;

    const int MAX_ITER = 3;

    while (err > 1e-10 && iter < MAX_ITER)
    {
        double last_miu = miu;

        beta = atan((1.0 - EARTH_FLATTENING) * tan(miu));
        sin_beta = sin(beta);
        cos_beta = cos(beta);
        miu = atan(
            (pos.z + EARTH_ECCENTRICITY_SQUARED * (1.0 - EARTH_FLATTENING) / (1.0 - EARTH_ECCENTRICITY_SQUARED) * EARTH_SEMI_MAJOR_AXIS * sin_beta * sin_beta * sin_beta) / (s - EARTH_ECCENTRICITY_SQUARED * EARTH_SEMI_MAJOR_AXIS * cos_beta * cos_beta * cos_beta));

        err = last_miu - miu;
        iter++;
    }

    double sin_miu = sin(miu);
    double n = EARTH_SEMI_MAJOR_AXIS / sqrt(1.0 - EARTH_ECCENTRICITY_SQUARED * sin_miu * sin_miu);
    double h = s * cos(miu) + (pos.z + EARTH_ECCENTRICITY_SQUARED * n * sin(miu)) * sin(miu) - n;

    return (geo_position_t){
        .lat = miu,
        .lon = atan2(pos.y, pos.x),
        .alt = h,
    };
}

static vec3_prec_t _ecef_to_ned(vec3_prec_t ecef, geo_position_t basePos)
{
    vec3_prec_t ecef0 = _geo_to_ecef(basePos);
    vec3_prec_t diff = {
        .x = ecef.x - ecef0.x,
        .y = ecef.y - ecef0.y,
        .z = ecef.z - ecef0.z,
    };

    double sin_lat = sin(basePos.lat);
    double cos_lat = cos(basePos.lat);
    double sin_lon = sin(basePos.lon);
    double cos_lon = cos(basePos.lon);

    return (vec3_prec_t){
        .x = diff.x * (-sin_lat * cos_lon) + diff.y * (-sin_lat * sin_lon) + diff.z * (+cos_lat),
        .y = diff.x * (-sin_lon) + diff.y * (+cos_lon),
        .z = diff.x * (-cos_lat * cos_lon) + diff.y * (-cos_lat * sin_lon) + diff.z * (-sin_lat),
    };
}

static vec3_prec_t _ned_to_ecef(vec3_prec_t ned, geo_position_t basePos)
{
    vec3_prec_t ecef0 = _geo_to_ecef(basePos);

    double sin_lat = sin(basePos.lat);
    double cos_lat = cos(basePos.lat);
    double sin_lon = sin(basePos.lon);
    double cos_lon = cos(basePos.lon);

    return (vec3_prec_t){
        .x = ecef0.x + ned.x * (-sin_lat * cos_lon) + ned.y * (-sin_lon) + ned.z * (-cos_lat * cos_lon),
        .y = ecef0.y + ned.x * (-sin_lat * sin_lon) + ned.y * (+cos_lon) + ned.z * (-cos_lat * sin_lon),
        .z = ecef0.z + ned.x * (cos_lat) + ned.z * (-sin_lat),
    };
}

vec3_prec_t wgs84_geo_to_ned(geo_position_t basePos, geo_position_t pos)
{
    _geo_pos_to_rad(&basePos);
    _geo_pos_to_rad(&pos);

    vec3_prec_t ecef = _geo_to_ecef(pos);

    return _ecef_to_ned(ecef, basePos);
}

geo_position_t wgs84_ned_to_geo(geo_position_t basePos, vec3_prec_t pos)
{
    _geo_pos_to_rad(&basePos);

    vec3_prec_t ecef = _ned_to_ecef(pos, basePos);
    geo_position_t geo = _ecef_to_geo(ecef);

    _geo_pos_to_deg(&geo);

    return geo;
}