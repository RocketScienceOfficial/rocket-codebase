#include <gtest/gtest.h>
#include <math.h>
#include <lib/maths/math_constants.h>

#include "../wgs84.h"
#include "../geo_utils.h"
#include "../geo_mag.h"
#include "../projection.h"
#include "../physical_constants.h"

TEST(GeoUtils, height_from_baro_formula_test)
{
    EXPECT_NEAR(height_from_baro_formula(101325), 0, 1);    // Sea level
    EXPECT_NEAR(height_from_baro_formula(89875), 1000, 10); // Approx. 1km
    EXPECT_NEAR(height_from_baro_formula(79500), 2000, 20); // Approx. 2km
}

TEST(Geo, geo_to_ned_and_back)
{
    geo_position_t basePos = {37.7749, -122.4194, 0}; // San Francisco
    geo_position_t pos = {34.0522, -118.2437, 0};     // Los Angeles

    vec3_prec_t ned = wgs84_geo_to_ned(basePos, pos);
    geo_position_t pos_converted = wgs84_ned_to_geo(basePos, ned);

    EXPECT_NEAR(pos.lat, pos_converted.lat, 1e-5);
    EXPECT_NEAR(pos.lon, pos_converted.lon, 1e-5);
    EXPECT_NEAR(pos.alt, pos_converted.alt, 1e-5);
}

TEST(Projection, zero_at_reference)
{
    geo_position_t ref = {45.0, 10.0, 100.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t ned = equirect_project_to_ned(&proj, &ref);

    EXPECT_NEAR(ned.x, 0.0f, 1e-3f);
    EXPECT_NEAR(ned.y, 0.0f, 1e-3f);
    EXPECT_NEAR(ned.z, 0.0f, 1e-3f);
}

TEST(Projection, north_offset)
{
    geo_position_t ref = {45.0, 10.0, 0.0};
    geo_position_t pos = {46.0, 10.0, 0.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t ned = equirect_project_to_ned(&proj, &pos);

    float expected_n = (float)(EARTH_RADIUS * 1.0 * DEG_2_RAD(1.0));
    EXPECT_NEAR(ned.x, expected_n, 1.0f);
    EXPECT_NEAR(ned.y, 0.0f, 1.0f);
    EXPECT_NEAR(ned.z, 0.0f, 1.0f);
}

TEST(Projection, east_offset)
{
    geo_position_t ref = {45.0, 10.0, 0.0};
    geo_position_t pos = {45.0, 11.0, 0.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t ned = equirect_project_to_ned(&proj, &pos);

    float expected_e = (float)(EARTH_RADIUS * cos(DEG_2_RAD(45.0)) * DEG_2_RAD(1.0));
    EXPECT_NEAR(ned.x, 0.0f, 1.0f);
    EXPECT_NEAR(ned.y, expected_e, 1.0f);
    EXPECT_NEAR(ned.z, 0.0f, 1.0f);
}

TEST(Projection, altitude_down)
{
    geo_position_t ref = {45.0, 10.0, 0.0};
    geo_position_t pos = {45.0, 10.0, 100.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t ned = equirect_project_to_ned(&proj, &pos);

    EXPECT_NEAR(ned.z, -100.0f, 0.01f);
}

TEST(Projection, round_trip)
{
    geo_position_t ref = {48.8566, 2.3522, 35.0};
    geo_position_t pos = {48.9000, 2.4000, 200.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t ned = equirect_project_to_ned(&proj, &pos);
    geo_position_t recovered = equirect_unproject_from_ned(&proj, &ned);

    EXPECT_NEAR(recovered.lat, pos.lat, 1e-5);
    EXPECT_NEAR(recovered.lon, pos.lon, 1e-5);
    EXPECT_NEAR(recovered.alt, pos.alt, 1e-3);
}

TEST(Projection, vs_geo_to_ned_short_range)
{
    geo_position_t ref = {37.7749, -122.4194, 0.0};
    geo_position_t pos = {37.7839, -122.4194, 0.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t eq = equirect_project_to_ned(&proj, &pos);
    vec3_prec_t precise = wgs84_geo_to_ned(ref, pos);

    EXPECT_NEAR(eq.x, (float)precise.x, 2.0f);
    EXPECT_NEAR(eq.y, (float)precise.y, 2.0f);
    EXPECT_NEAR(eq.z, (float)precise.z, 2.0f);
}

TEST(Projection, vs_geo_to_ned_medium_range)
{
    geo_position_t ref = {37.7749, -122.4194, 0.0};
    geo_position_t pos = {38.2249, -122.4194, 0.0};
    equirect_projection_t proj;
    equirect_projection_init(&proj, &ref);

    vec3_t eq = equirect_project_to_ned(&proj, &pos);
    vec3_prec_t precise = wgs84_geo_to_ned(ref, pos);

    EXPECT_NEAR(eq.x, (float)precise.x, 100.0f);
    EXPECT_NEAR(eq.y, (float)precise.y, 100.0f);
    EXPECT_NEAR(eq.z, (float)precise.z, 250.0f);
}

TEST(Mag, field_vector)
{
    geo_position_t pos = {51.486150, 15.732828, 0.0};
    vec3_t magField = geo_mag_field_vector(&pos);

    EXPECT_NEAR(magField.x, 0.19073f, 0.002f);
    EXPECT_NEAR(magField.y, 0.01869f, 0.002f);
    EXPECT_NEAR(magField.z, 0.46160f, 0.002f);
}