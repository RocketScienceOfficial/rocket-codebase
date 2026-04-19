#include <gtest/gtest.h>
#include <math.h>
#include "../geo.h"
#include "../geo_utils.h"
#include "../geo_mag.h"

TEST(GeoUtils, height_from_baro_formula_test)
{
    EXPECT_NEAR(height_from_baro_formula(101325), 0, 1);    // Sea level
    EXPECT_NEAR(height_from_baro_formula(89875), 1000, 10); // Approx. 1km
    EXPECT_NEAR(height_from_baro_formula(79500), 2000, 20); // Approx. 2km
}

TEST(Geo, geo_to_ned_and_back)
{
    geo_position_wgs84_t basePos = {37.7749, -122.4194, 0}; // San Francisco
    geo_position_wgs84_t pos = {34.0522, -118.2437, 0};     // Los Angeles

    vec3_prec_t ned = geo_to_ned(basePos, pos);
    geo_position_wgs84_t pos_converted = ned_to_geo(basePos, ned);

    EXPECT_NEAR(pos.lat, pos_converted.lat, 1e-5);
    EXPECT_NEAR(pos.lon, pos_converted.lon, 1e-5);
    EXPECT_NEAR(pos.alt, pos_converted.alt, 1e-5);
}

TEST(Geo, geo_calculate_distance)
{
    geo_position_t p0 = {37.7749, -122.4194}; // San Francisco
    geo_position_t p1 = {34.0522, -118.2437}; // Los Angeles

    float distance = geo_calculate_distance(p0, p1);
    EXPECT_NEAR(distance, 559000, 1000); // Approx. distance in meters
}

TEST(Geo, geo_calculate_bearing)
{
    geo_position_t p0 = {37.7749, -122.4194}; // San Francisco
    geo_position_t p1 = {34.0522, -118.2437}; // Los Angeles

    float bearing = geo_calculate_bearing(p0, p1);
    EXPECT_NEAR(bearing, 135, 5); // Approx. bearing in degrees
}