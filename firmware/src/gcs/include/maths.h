#pragma once

#include <Arduino.h>

#define RAD_2_DEG(x) ((x) * 180.0 / PI)
#define DEG_2_RAD(x) ((x) * PI / 180.0)
#define EARTH_RADIUS 6371000.0

float calculate_geo_distance(float lat1, float lon1, float lat2, float lon2);