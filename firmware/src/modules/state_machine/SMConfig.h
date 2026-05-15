#pragma once

#include <lib/geo/physical_constants.h>

/** Barometer */
#define SM_CFG_BARO_RATE 50
#define SM_CFG_BARO_SMOOTHING_ALPHA 0.2f

/** Start Verification */
#define SM_CFG_START_ACC_THRESHOLD (2.5f * EARTH_GRAVITY) // Compared to raw acceleration, so effectively 1g less
#define SM_CFG_START_ALT_THRESHOLD 1.0f                   // Compared to temporary base altitude
#define SM_CFG_START_ALT_VERIFICATION_TIME_MS 500         // Max time to verify altitude after acceleration threshold is exceeded
#define SM_CFG_START_ALT_FALLBACK_HEIGHT 10.0f            // If not detected via last conditions, this acts like a fallback altitude to detect start of flight. This is to prevent cases where the rocket is launched from a moving platform and the acceleration threshold is never exceeded.

/** Apogee Verification */
#define SM_CFG_APOGEE_MAX_DELTA 0.5f       // Max allowed altitude difference from detected apogee to consider it valid
#define SM_CFG_LAST_ALT_APOGEE_TIME_MS 250 // Time after detected apogee to verify that altitude stays within max delta

/** Landing Verification */
#define SM_CFG_LAND_MAX_DELTA 1.0f                     // Max allowed altitude difference from detected landing altitude to consider it valid
#define SM_CFG_LAST_ALT_LAND_VERIFICATION_TIME_MS 4000 // Time after detected landing altitude to verify that altitude stays within max delta