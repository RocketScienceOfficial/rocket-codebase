#include "madgwick.h"
#include <math.h>

void madgwick_update_imu(quat_t *q, float dt, float beta, vec3_t gyroVec, vec3_t accVec)
{
    vec3_normalize(&accVec);

    const float _2qw = 2.0f * q->w;
    const float _2qx = 2.0f * q->x;
    const float _2qy = 2.0f * q->y;
    const float _2qz = 2.0f * q->z;

    const vec3_t func_g = (vec3_t){
        _2qx * q->z - _2qw * q->y - accVec.x,
        _2qw * q->x + _2qy * q->z - accVec.y,
        1.0f - _2qx * q->x - _2qy * q->y - accVec.z,
    };

    quat_t grad = (quat_t){
        -_2qy * func_g.x + _2qx * func_g.y,
        +_2qz * func_g.x + _2qw * func_g.y - 2.0f * _2qx * func_g.z,
        -_2qw * func_g.x + _2qz * func_g.y - 2.0f * _2qy * func_g.z,
        +_2qx * func_g.x + _2qy * func_g.y,
    };
    quat_normalize(&grad);

    quat_t derivative = quat_gyro_derivative(q, &gyroVec);
    derivative.w -= beta * grad.w;
    derivative.x -= beta * grad.x;
    derivative.y -= beta * grad.y;
    derivative.z -= beta * grad.z;

    q->w += derivative.w * dt;
    q->x += derivative.x * dt;
    q->y += derivative.y * dt;
    q->z += derivative.z * dt;

    quat_normalize(q);
}

void madgwick_update_marg(quat_t *q, float dt, float beta, vec3_t gyroVec, vec3_t accVec, vec3_t magVec)
{
    vec3_normalize(&accVec);
    vec3_normalize(&magVec);

    const float _2qw = 2 * q->w;
    const float _2qx = 2 * q->x;
    const float _2qy = 2 * q->y;
    const float _2qz = 2 * q->z;
    const float qwqw = q->w * q->w;
    const float qxqx = q->x * q->x;
    const float qyqy = q->y * q->y;
    const float qzqz = q->z * q->z;
    const float qxqy = q->x * q->y;
    const float qxqz = q->x * q->z;
    const float qwqy = q->w * q->y;
    const float qwqz = q->w * q->z;
    const float qyqz = q->y * q->z;
    const float qwqx = q->w * q->x;
    const float hx = qxqx * magVec.x + 2 * qxqy * magVec.z + 2 * qxqz * magVec.z + qwqw * magVec.x + 2 * qwqy * magVec.z - 2 * qwqz * magVec.y - qzqz * magVec.x - qyqy * magVec.x;
    const float hy = 2 * qxqy * magVec.x + qyqy * magVec.y + 2 * qyqz * magVec.z + 2 * qwqz * magVec.x - qzqz * magVec.y - qwqw * magVec.y - 2 * qwqx * magVec.z - qxqx * magVec.y;
    const float _2bx = 2 * sqrtf(hx * hx + hy * hy);
    const float _2bz = 2 * (2 * qxqz * magVec.x + 2 * qyqz * magVec.y + qzqz * magVec.z - 2 * qwqy * magVec.x - qyqy * magVec.z + 2 * qwqx * magVec.y - qxqx * magVec.z + qwqw * magVec.z);
    const float _2bxqz = _2bx * q->z;
    const float _2bxqy = _2bx * q->y;
    const float _2bzqx = _2bz * q->x;
    const float _2bzqy = _2bz * q->y;

    const vec3_t func_g = (vec3_t){
        _2qx * q->z - _2qw * q->y - accVec.x,
        _2qw * q->x + _2qy * q->z - accVec.y,
        1.0f - _2qx * q->x - _2qy * q->y - accVec.z,
    };
    const vec3_t func_b = (vec3_t){
        _2bx * (0.5f - qyqy - qzqz) + _2bz * (qxqz - qwqy) - magVec.x,
        _2bx * (qxqy - qwqz) + _2bz * (qwqx + qyqz) - magVec.y,
        _2bx * (qwqy + qxqz) + _2bz * (0.5f - qxqx - qyqy) - magVec.z,
    };

    quat_t grad = (quat_t){
        -_2qy * func_g.x + _2qx * func_g.y - _2bzqy * func_b.x + (-_2bxqz + _2bzqx) * func_b.y + _2bxqy * func_b.z,
        +_2qz * func_g.x + _2qw * func_g.y - 2 * _2qx * func_g.z + _2bz * q->z * func_b.x + (_2bxqy + _2bz * q->w) * func_b.y + (_2bxqz - 2 * _2bzqx) * func_b.z,
        -_2qw * func_g.x + _2qz * func_g.y - 2 * _2qy * func_g.z + (-2 * _2bxqy - _2bz * q->w) * func_b.x + (_2bx * q->x + _2bz * q->z) * func_b.y + (_2bx * q->w - 2 * _2bzqy) * func_b.z,
        +_2qx * func_g.x + _2qy * func_g.y + (-2 * _2bxqz + _2bzqx) * func_b.x + (-_2bx * q->w + _2bzqy) * func_b.y + _2bx * q->x * func_b.z,
    };
    quat_normalize(&grad);

    quat_t derivative = quat_gyro_derivative(q, &gyroVec);
    derivative.w -= beta * grad.w;
    derivative.x -= beta * grad.x;
    derivative.y -= beta * grad.y;
    derivative.z -= beta * grad.z;

    q->w += derivative.w * dt;
    q->x += derivative.x * dt;
    q->y += derivative.y * dt;
    q->z += derivative.z * dt;

    quat_normalize(q);
}