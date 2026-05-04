#include "quaternion.h"
#include "math_constants.h"
#include "fast_math.h"
#include "math_utils.h"
#include <lib/debug/sys_assert.h>
#include <math.h>

quat_t quat_mul(const quat_t *a, const quat_t *b)
{
    SYS_ASSERT(a != NULL);
    SYS_ASSERT(b != NULL);

    return (quat_t){
        .w = a->w * b->w - a->x * b->x - a->y * b->y - a->z * b->z,
        .x = a->w * b->x + a->x * b->w + a->y * b->z - a->z * b->y,
        .y = a->w * b->y - a->x * b->z + a->y * b->w + a->z * b->x,
        .z = a->w * b->z + a->x * b->y - a->y * b->x + a->z * b->w,
    };
}

quat_t quat_conj(const quat_t *q)
{
    SYS_ASSERT(q != NULL);

    return (quat_t){
        .w = q->w,
        .x = -q->x,
        .y = -q->y,
        .z = -q->z,
    };
}

void quat_inv(quat_t *q)
{
    SYS_ASSERT(q != NULL);

    float sq_norm = q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z;
    float normInv = fast_inv_sqrt(sq_norm);
    float normInv2 = normInv * normInv;

    q->w *= normInv2;
    q->x *= -normInv2;
    q->y *= -normInv2;
    q->z *= -normInv2;
}

void quat_normalize(quat_t *q)
{
    SYS_ASSERT(q != NULL);

    float sq_norm = q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z;
    float normInv = fast_inv_sqrt(sq_norm);

    q->w *= normInv;
    q->x *= normInv;
    q->y *= normInv;
    q->z *= normInv;
}

void quat_rotate_vec(vec3_t *v, const quat_t *q)
{
    SYS_ASSERT(v != NULL);
    SYS_ASSERT(q != NULL);

    *v = (vec3_t){
        .x = v->x * (1.0f - 2.0f * (q->y * q->y + q->z * q->z)) + v->y * 2.0f * (q->x * q->y - q->z * q->w) + v->z * 2.0f * (q->x * q->z + q->y * q->w),
        .y = v->x * 2.0f * (q->x * q->y + q->z * q->w) + v->y * (1.0f - 2.0f * (q->x * q->x + q->z * q->z)) + v->z * 2.0f * (q->y * q->z - q->x * q->w),
        .z = v->x * 2.0f * (q->x * q->z - q->y * q->w) + v->y * 2.0f * (q->y * q->z + q->x * q->w) + v->z * (1.0f - 2.0f * (q->x * q->x + q->y * q->y)),
    };
}

quat_t quat_from_vecs(const vec3_t *from, const vec3_t *to)
{
    SYS_ASSERT(from != NULL);
    SYS_ASSERT(to != NULL);

    float dot = vec3_dot(from, to);
    vec3_t crs = vec3_cross(from, to);

    quat_t result = {
        .w = 1 + dot,
        .x = crs.x,
        .y = crs.y,
        .z = crs.z,
    };
    quat_normalize(&result);

    return result;
}

quat_t quat_gyro_derivative(const quat_t *q, const vec3_t *gyro)
{
    SYS_ASSERT(q != NULL);
    SYS_ASSERT(gyro != NULL);

    return (quat_t){
        .w = 0.5f * (-gyro->x * q->x - gyro->y * q->y - gyro->z * q->z),
        .x = 0.5f * (+gyro->x * q->w + gyro->z * q->y - gyro->y * q->z),
        .y = 0.5f * (+gyro->y * q->w - gyro->z * q->x + gyro->x * q->z),
        .z = 0.5f * (+gyro->z * q->w + gyro->y * q->x - gyro->x * q->y),
    };
}

void integrate_gyro(quat_t *q, vec3_t gyro, float dt)
{
    SYS_ASSERT(q != NULL);

    quat_t derivative = quat_gyro_derivative(q, &gyro);

    q->w += derivative.w * dt;
    q->x += derivative.x * dt;
    q->y += derivative.y * dt;
    q->z += derivative.z * dt;

    quat_normalize(q);
}