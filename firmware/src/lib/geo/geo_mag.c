#include "geo_mag.h"
#include "wmm/mag_tables.h"
#include <lib/debug/sys_assert.h>
#include <stdint.h>
#include <math.h>

static float _get_value_from_table(const geo_position_t* pos, const int16_t table[LAT_DIM][LON_DIM])
{
    SYS_ASSERT(pos->lat >= SAMPLING_MIN_LAT);
    SYS_ASSERT(pos->lat <= SAMPLING_MAX_LAT);
    SYS_ASSERT(pos->lon >= SAMPLING_MIN_LON);
    SYS_ASSERT(pos->lon <= SAMPLING_MAX_LON);

    float min_lat = floorf(pos->lat / SAMPLING_RES) * SAMPLING_RES;
    float min_lon = floorf(pos->lon / SAMPLING_RES) * SAMPLING_RES;

    int min_lat_index = (int)((min_lat - SAMPLING_MIN_LAT) / SAMPLING_RES);
    int min_lon_index = (int)((min_lon - SAMPLING_MIN_LON) / SAMPLING_RES);

    SYS_ASSERT(min_lat_index >= 0);
    SYS_ASSERT(min_lat_index + 1 < LAT_DIM);
    SYS_ASSERT(min_lon_index >= 0);
    SYS_ASSERT(min_lon_index + 1 < LON_DIM);

    float data_ne = (float)table[min_lat_index + 1][min_lon_index + 1];
    float data_se = (float)table[min_lat_index][min_lon_index + 1];
    float data_sw = (float)table[min_lat_index][min_lon_index];
    float data_nw = (float)table[min_lat_index + 1][min_lon_index];

    float data_min = (data_se - data_sw) / SAMPLING_RES * (pos->lon - min_lon) + data_sw;
    float data_max = (data_ne - data_nw) / SAMPLING_RES * (pos->lon - min_lon) + data_nw;
    float data = (data_max - data_min) / SAMPLING_RES * (pos->lat - min_lat) + data_min;

    return data / 10000.0f;
}

float geo_mag_get_declination(const geo_position_t* pos)
{
    return _get_value_from_table(pos, DECLINATION_TABLE);
}

float geo_mag_get_inclination(const geo_position_t* pos)
{
    return _get_value_from_table(pos, INCLINATION_TABLE);
}

float geo_mag_get_strength(const geo_position_t* pos)
{
    return _get_value_from_table(pos, STRENGTH_TABLE);
}

vec3_t geo_mag_field_vector(const geo_position_t* pos)
{
    float s = geo_mag_get_strength(pos);
    float i = geo_mag_get_inclination(pos);
    float d = geo_mag_get_declination(pos);

    float cos_i = cosf(i);

    return (vec3_t){
        .x = s * cos_i * cosf(d),
        .y = s * cos_i * sinf(d),
        .z = s * sinf(i),
    };
}