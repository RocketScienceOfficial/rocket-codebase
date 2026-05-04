#include <gtest/gtest.h>
#include <math.h>

#include "../vector.h"

TEST(Vector, magnitude)
{
    vec3_t v = {.x = 3.0f, .y = 4.0f, .z = 12.0f};
    EXPECT_NEAR(vec3_mag(&v), 13.0f, 1e-6f);
}

TEST(Vector, mag_compare)
{
    vec3_t v = {.x = 3.0f, .y = 4.0f, .z = 12.0f}; // mag = 13

    EXPECT_EQ(vec3_mag_compare(&v, 13.0f), 0);
    EXPECT_EQ(vec3_mag_compare(&v, 12.0f), 1);
    EXPECT_EQ(vec3_mag_compare(&v, 14.0f), -1);
}

TEST(Vector, normalize)
{
    vec3_t v = {.x = 0.0f, .y = 3.0f, .z = 4.0f}; // mag = 5
    vec3_normalize(&v);

    EXPECT_NEAR(v.x, 0.0f, 1e-4f);
    EXPECT_NEAR(v.y, 0.6f, 1e-4f);
    EXPECT_NEAR(v.z, 0.8f, 1e-4f);
    EXPECT_NEAR(vec3_mag(&v), 1.0f, 1e-4f);
}

TEST(Vector, dot_product)
{
    vec3_t a = {.x = 1.0f, .y = 2.0f, .z = 3.0f};
    vec3_t b = {.x = 4.0f, .y = -5.0f, .z = 6.0f};

    EXPECT_NEAR(vec3_dot(&a, &b), 12.0f, 1e-6f);
}

TEST(Vector, cross_product)
{
    vec3_t i = {.x = 1.0f, .y = 0.0f, .z = 0.0f};
    vec3_t j = {.x = 0.0f, .y = 1.0f, .z = 0.0f};
    vec3_t k = vec3_cross(&i, &j);

    EXPECT_NEAR(k.x, 0.0f, 1e-6f);
    EXPECT_NEAR(k.y, 0.0f, 1e-6f);
    EXPECT_NEAR(k.z, 1.0f, 1e-6f);

    vec3_t a = {.x = 1.0f, .y = 2.0f, .z = 3.0f};
    vec3_t b = {.x = 4.0f, .y = 5.0f, .z = 6.0f};
    vec3_t c = vec3_cross(&a, &b);

    EXPECT_NEAR(c.x, -3.0f, 1e-6f);
    EXPECT_NEAR(c.y, 6.0f, 1e-6f);
    EXPECT_NEAR(c.z, -3.0f, 1e-6f);
}