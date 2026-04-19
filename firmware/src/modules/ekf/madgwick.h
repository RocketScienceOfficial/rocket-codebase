#ifndef _MADGWICK_H
#define _MADGWICK_H

/**
 * REF: https://ahrs.readthedocs.io/en/latest/filters/madgwick.html
 * REF: https://github.com/adafruit/Adafruit_AHRS/blob/master/src/Adafruit_AHRS_Madgwick.cpp
 * REF: https://github.com/arduino-libraries/MadgwickAHRS
 */

#include <lib/maths/quaternion.h>
#include <lib/maths/vector.h>

#ifdef __cplusplus
extern "C" {
#endif

void madgwick_update_imu(quat_t *q, float dt, float beta, vec3_t gyroVec, vec3_t accVec);
void madgwick_update_marg(quat_t *q, float dt, float beta, vec3_t gyroVec, vec3_t accVec, vec3_t magVec);

#ifdef __cplusplus
}
#endif

#endif