#include <gtest/gtest.h>
#include <math.h>
#include "lib/maths/fast_math.h"
#include "lib/maths/quaternion.h"

TEST(FastMath, fast_inv_sqrt_test)
{
    float actual = fast_inv_sqrt(2.0f);
    float expected = 1.0f / sqrt(2.0f);

    EXPECT_NEAR(actual, expected, 1e-4f);
}

TEST(FastMath, fast_modulo_test)
{
    int x = 10;
    int m = 4;
    int actual = FAST_MODULO(x, m);
    int expected = x % m;

    EXPECT_EQ(actual, expected);
}

TEST(Quaternion, quat_mul_test)
{
    quat_t a = {1.0f, 0.0f, 0.0f, 0.0f};
    quat_t b = {0.0f, 1.0f, 0.0f, 0.0f};
    quat_t result = quat_mul(&a, &b);

    EXPECT_FLOAT_EQ(result.w, 0.0f);
    EXPECT_FLOAT_EQ(result.x, 1.0f);
    EXPECT_FLOAT_EQ(result.y, 0.0f);
    EXPECT_FLOAT_EQ(result.z, 0.0f);
}

TEST(Quaternion, quat_inv_test)
{
    quat_t q = {1.0f, 2.0f, 3.0f, 4.0f};
    quat_inv(&q);

    EXPECT_NEAR(q.w, +0.033f, 1e-3f);
    EXPECT_NEAR(q.x, -0.066f, 1e-3f);
    EXPECT_NEAR(q.y, -0.100f, 1e-3f);
    EXPECT_NEAR(q.z, -0.133f, 1e-3f);
}

TEST(Quaternion, quat_normalize_test)
{
    quat_t q = {1.0f, 2.0f, 3.0f, 4.0f};
    quat_normalize(&q);

    EXPECT_NEAR(q.w, 0.182f, 1e-3f);
    EXPECT_NEAR(q.x, 0.365f, 1e-3f);
    EXPECT_NEAR(q.y, 0.547f, 1e-3f);
    EXPECT_NEAR(q.z, 0.730f, 1e-3f);
}

TEST(Quaternion, quat_rotate_vec_test)
{
    quat_t q = {0.7071f, 0.7071f, 0.0f, 0.0f}; // 90 degree rotation around x-axis
    vec3_t v = {0.0f, 1.0f, 0.0f};
    quat_rotate_vec(&v, &q);

    EXPECT_NEAR(v.x, 0.0f, 1e-4f);
    EXPECT_NEAR(v.y, 0.0f, 1e-4f);
    EXPECT_NEAR(v.z, 1.0f, 1e-4f);
}

TEST(Quaternion, quat_gyro_derivative_test)
{
    quat_t q = {0.7071f, 0.0f, 0.0f, 0.7071f};
    vec3_t gyro = {1.0f, 0.0f, 0.0f};
    quat_t derivative = quat_gyro_derivative(&q, &gyro);

    EXPECT_NEAR(derivative.w, 0.0f, 1e-3f);
    EXPECT_NEAR(derivative.x, 0.354f, 1e-3f);
    EXPECT_NEAR(derivative.y, 0.354f, 1e-3f);
    EXPECT_NEAR(derivative.z, 0.0f, 1e-3f);
}