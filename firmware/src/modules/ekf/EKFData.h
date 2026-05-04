#pragma once

#include <lib/maths/vector.h>
#include <lib/maths/quaternion.h>

#define EKF_NUM_NOMINAL_STATES 16
#define EKF_NUM_ERROR_STATES 15
#define EKF_NUM_INPUTS 6

struct EKFIMUData
{
    vec3_t delta_velocity;
    vec3_t delta_angle;
    float varAcc;
    float varGyro;
    float dt;
};

struct EKFBaroMeasurement
{
    float height;
    float var;
};

struct EKFGPSPosMeasurement
{
    vec3_t pos;
    float var;
};

struct EKFGPSVelMeasurement
{
    vec3_t vel;
    float var;
};

struct EKFNominalState
{
    quat_t attitude;
    vec3_t pos;
    vec3_t vel;
    vec3_t bias_gyro;
    vec3_t bias_acc;

    float *asArray() { return reinterpret_cast<float *>(this); }
    const float *asArray() const { return reinterpret_cast<const float *>(this); }
};

static_assert(sizeof(EKFNominalState) == EKF_NUM_NOMINAL_STATES * sizeof(float), "EKFNominalState size mismatch");

struct EKFErrorState
{
    vec3_t theta;
    vec3_t pos;
    vec3_t vel;
    vec3_t bias_gyro;
    vec3_t bias_acc;

    float *asArray() { return reinterpret_cast<float *>(this); }
    const float *asArray() const { return reinterpret_cast<const float *>(this); }
};

static_assert(sizeof(EKFErrorState) == EKF_NUM_ERROR_STATES * sizeof(float), "EKFErrorState size mismatch");