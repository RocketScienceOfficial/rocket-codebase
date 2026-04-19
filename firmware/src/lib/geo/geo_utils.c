#include "geo_utils.h"
#include "physical_constants.h"
#include <lib/debug/obc_assert.h>
#include <math.h>

#define EXP_CONSTANT 0.1902632

float height_from_baro_formula(int pressure)
{
    OBC_ASSERT(pressure > 0);

    return SEA_LEVEL_TEMPERATURE / STANDARD_LAPSE_RATE * (1.0f - powf((float)pressure / (float)SEA_LEVEL_PRESSURE, (float)EXP_CONSTANT));
}