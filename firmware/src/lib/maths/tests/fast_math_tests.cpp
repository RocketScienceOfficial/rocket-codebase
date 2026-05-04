#include <gtest/gtest.h>
#include <math.h>

#include "../fast_math.h"

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