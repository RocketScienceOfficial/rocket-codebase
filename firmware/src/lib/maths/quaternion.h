#ifndef _QUATERNION_H
#define _QUATERNION_H

#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Quaternion data structure
 */
typedef struct
{
    float w, x, y, z;
} quat_t;

/**
 * @brief Multiply quaternions
 *
 * @param a First quaternion
 * @param b Second quaternion
 * @return Result of multiplication
 */
quat_t quat_mul(const quat_t *a, const quat_t *b);

/**
 * @brief Conjugate of a quaternion
 *
 * @param q Quaternion to conjugate
 * @return Conjugated quaternion
 */
quat_t quat_conj(const quat_t *q);

/**
 * @brief Calculate inverse of a quaternion (using fast inverse square root)
 *
 * @param q Quaternion to inverse
 */
void quat_inv(quat_t *q);

/**
 * @brief Normalize a quaternion (using fast inverse square root)
 *
 * @param q Quaternion to normalize
 */
void quat_normalize(quat_t *q);

/**
 * @brief Rotate vector through quaternion
 *
 * @param v Vector to rotate
 * @param q Quaternion
 */
void quat_rotate_vec(vec3_t *v, const quat_t *q);

/**
 * @brief Convert quaternion to Euler angles (roll, pitch, yaw)
 * 
 * @param q Quaternion to convert
 * @return Euler angles in radians (x=roll, y=pitch, z=yaw)
 */
vec3_t quat_to_euler(const quat_t *q);

/**
 * @brief Convert Euler angles (roll, pitch, yaw) to quaternion
 * 
 * @param roll Roll angle in radians
 * @param pitch Pitch angle in radians
 * @param yaw Yaw angle in radians
 * @return Quaternion representing the given Euler angles (normalized)
 */
quat_t quat_from_euler(float roll, float pitch, float yaw);

/**
 * @brief Calculate quaternion representing rotation from one vector to another
 * 
 * @param from Starting vector
 * @param to Target vector
 * @return Quaternion representing rotation from "from" to "to" (normalized)
 */
quat_t quat_from_vecs(const vec3_t *from, const vec3_t *to);

/**
 * @brief Calculate quaternion representing orientation from accelerometer and magnetometer readings. "acc" should represent the gravity vector, and "mag" should represent the magnetic field vector in the body frame.
 * 
 * @param acc Accelerometer vector (FRD frame)
 * @param mag Magnetometer vector (FRD frame)
 * @return Quaternion representing orientation (normalized)
 */
quat_t quat_from_acc_mag(const vec3_t *acc, const vec3_t *mag);

/**
 * @brief Calculate derivative of quaternion
 *
 * @param q Quaternion
 * @param gyro Gyroscope vector
 * @return Derivative of quaternion
 */
quat_t quat_gyro_derivative(const quat_t *q, const vec3_t *gyro);

/**
 * @brief Integrate gyroscope data into quaternion. See: https://ahrs.readthedocs.io/en/latest/filters/angular.html
 * 
 * @param q Quaternion to integrate into
 * @param gyro Gyroscope vector
 * @param dt Time step
 */
void integrate_gyro(quat_t *q, vec3_t gyro, float dt);

#ifdef __cplusplus
}
#endif

#endif