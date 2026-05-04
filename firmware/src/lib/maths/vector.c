#include "vector.h"
#include "fast_math.h"
#include <lib/debug/sys_assert.h>
#include <math.h>

vec3_t vec3_add(const vec3_t *a, const vec3_t *b)
{
    SYS_ASSERT(a != NULL);
    SYS_ASSERT(b != NULL);

    return (vec3_t){
        .x = a->x + b->x,
        .y = a->y + b->y,
        .z = a->z + b->z,
    };
}

vec3_t vec3_sub(const vec3_t *a, const vec3_t *b)
{
    SYS_ASSERT(a != NULL);
    SYS_ASSERT(b != NULL);

    return (vec3_t){
        .x = a->x - b->x,
        .y = a->y - b->y,
        .z = a->z - b->z,
    };
}

void vec3_mul_num(vec3_t *v, float n)
{
    SYS_ASSERT(v != NULL);

    v->x *= n;
    v->y *= n;
    v->z *= n;
}

float vec3_mag(const vec3_t *v)
{
    SYS_ASSERT(v != NULL);

    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

int vec3_mag_compare(const vec3_t *v, float n)
{
    SYS_ASSERT(v != NULL);

    float r = v->x * v->x + v->y * v->y + v->z * v->z;

    return r > n * n ? 1 : r < n * n ? -1
                                     : 0;
}

void vec3_normalize(vec3_t *v)
{
    SYS_ASSERT(v != NULL);

    float sq_mag = v->x * v->x + v->y * v->y + v->z * v->z;
    float magInv = fast_inv_sqrt(sq_mag);

    v->x *= magInv;
    v->y *= magInv;
    v->z *= magInv;
}

float vec3_dot(const vec3_t *a, const vec3_t *b)
{
    SYS_ASSERT(a != NULL);
    SYS_ASSERT(b != NULL);

    return a->x * b->x + a->y * b->y + a->z * b->z;
}

vec3_t vec3_cross(const vec3_t *a, const vec3_t *b)
{
    SYS_ASSERT(a != NULL);
    SYS_ASSERT(b != NULL);

    return (vec3_t){
        .x = a->y * b->z - a->z * b->y,
        .y = a->z * b->x - a->x * b->z,
        .z = a->x * b->y - a->y * b->x,
    };
}