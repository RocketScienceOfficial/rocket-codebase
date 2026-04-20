#include "math_utils.h"
#include <lib/debug/sys_assert.h>

float clamp_value(float x, float min, float max)
{
    SYS_ASSERT(min <= max);

    return x > max ? max : (x < min ? min : x);
}

bool value_approx_eql(float val, float des, float eps)
{
    SYS_ASSERT(eps >= 0.0f);

    return val >= des - eps && val <= des + eps;
}

float exp_smoothing(float x1, float x0, float a)
{
    SYS_ASSERT(a >= 0.0f && a <= 1.0f);

    return x0 == 0 ? x1 : a * x1 + (1 - a) * x0;
}