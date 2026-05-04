#include "EKF.h"
#include "modules/common/ModuleLogger.h"
#include "derivation/generated/covariance_prediction.h"
#include "derivation/generated/baro_fusion.h"
#include "derivation/generated/gps_fusion_pos_n.h"
#include "derivation/generated/gps_fusion_pos_e.h"
#include "derivation/generated/gps_fusion_pos_d.h"
#include "derivation/generated/gps_fusion_vel_n.h"
#include "derivation/generated/gps_fusion_vel_e.h"
#include "derivation/generated/gps_fusion_vel_d.h"
#include <lib/geo/physical_constants.h>
#include <lib/debug/sys_assert.h>

void EKF::init()
{
    resetErrorState();

    P_current.resetCompletely();
    P_next.resetCompletely();
}

void EKF::predictExplicitState(EKFNominalState &state, const EKFErrorState &error, const EKFIMUData &imu)
{
    const float &dt = imu.dt;

    state.bias_acc.x += error.bias_acc.x * dt;
    state.bias_acc.y += error.bias_acc.y * dt;
    state.bias_acc.z += error.bias_acc.z * dt;

    state.bias_gyro.x += error.bias_gyro.x * dt;
    state.bias_gyro.y += error.bias_gyro.y * dt;
    state.bias_gyro.z += error.bias_gyro.z * dt;

    state.pos.x += (state.vel.x + error.pos.x) * dt;
    state.pos.y += (state.vel.y + error.pos.y) * dt;
    state.pos.z += (state.vel.z + error.pos.z) * dt;

    vec3_t dv = {
        .x = imu.delta_velocity.x - state.bias_acc.x * dt,
        .y = imu.delta_velocity.y - state.bias_acc.y * dt,
        .z = imu.delta_velocity.z - state.bias_acc.z * dt,
    };
    quat_rotate_vec(&dv, &state.attitude);

    state.vel.x += dv.x + error.vel.x * dt;
    state.vel.y += dv.y + error.vel.y * dt;
    state.vel.z += dv.z + error.vel.z * dt + EARTH_GRAVITY * dt;

    quat_t dq = {
        .w = 1.0f,
        .x = 0.5f * (imu.delta_angle.x + error.theta.x * dt - state.bias_gyro.x * dt),
        .y = 0.5f * (imu.delta_angle.y + error.theta.y * dt - state.bias_gyro.y * dt),
        .z = 0.5f * (imu.delta_angle.z + error.theta.z * dt - state.bias_gyro.z * dt),
    };
    state.attitude = quat_mul(&state.attitude, &dq);
    quat_normalize(&state.attitude);
}

void EKF::predictState(const EKFIMUData &imu)
{
    SYS_ASSERT(m_ErrorState.asArray()[0] == 0.0f); // Sanity check that error state is zero

    // Error state is always 0 here
    predictExplicitState(m_NominalState, m_ErrorState, imu);
}

void EKF::predictCovariance(const EKFIMUData &imu)
{
    float acc_x = imu.delta_velocity.x / imu.dt;
    float acc_y = imu.delta_velocity.y / imu.dt;
    float acc_z = imu.delta_velocity.z / imu.dt;
    float gyro_x = imu.delta_angle.x / imu.dt;
    float gyro_y = imu.delta_angle.y / imu.dt;
    float gyro_z = imu.delta_angle.z / imu.dt;

    gen::covariance_prediction(m_NominalState.asArray(), P_current, P_next, imu.varAcc, acc_x, acc_y, acc_z, imu.varGyro, gyro_x, gyro_y, gyro_z, imu.dt);

    // Copy upper triangle of P_next to P_current
    for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
    {
        for (size_t j = i; j < EKF_NUM_ERROR_STATES; j++)
        {
            P_current(i, j) = P_next(i, j);
        }
    }
}

bool EKF::fuseGPSPosition(const EKFGPSPosMeasurement &meas, float gate_threshold)
{
    float innov, innov_var;

    gen::gps_fusion_pos_n(m_NominalState.asArray(), P_current, meas.pos.x, meas.var_hor, &innov, &innov_var, _H, _K);
    if (!applyFusion(innov, innov_var, gate_threshold))
    {
        return false;
    }

    gen::gps_fusion_pos_e(m_NominalState.asArray(), P_current, meas.pos.y, meas.var_hor, &innov, &innov_var, _H, _K);
    if (!applyFusion(innov, innov_var, gate_threshold))
    {
        return false;
    }

    gen::gps_fusion_pos_d(m_NominalState.asArray(), P_current, meas.pos.z, meas.var_ver, &innov, &innov_var, _H, _K);
    if (!applyFusion(innov, innov_var, gate_threshold))
    {
        return false;
    }

    return true;
}

bool EKF::fuseGPSVelocity(const EKFGPSVelMeasurement &meas, float gate_threshold)
{
    float innov, innov_var;

    gen::gps_fusion_vel_n(m_NominalState.asArray(), P_current, meas.vel.x, meas.var, &innov, &innov_var, _H, _K);
    if (!applyFusion(innov, innov_var, gate_threshold))
    {
        return false;
    }

    gen::gps_fusion_vel_e(m_NominalState.asArray(), P_current, meas.vel.y, meas.var, &innov, &innov_var, _H, _K);
    if (!applyFusion(innov, innov_var, gate_threshold))
    {
        return false;
    }

    gen::gps_fusion_vel_d(m_NominalState.asArray(), P_current, meas.vel.z, meas.var, &innov, &innov_var, _H, _K);
    if (!applyFusion(innov, innov_var, gate_threshold))
    {
        return false;
    }

    return true;
}

bool EKF::fuseBaroHeight(const EKFBaroMeasurement &meas, float gate_threshold)
{
    float innov, innov_var;

    gen::baro_fusion(m_NominalState.asArray(), P_current, meas.height, meas.var, &innov, &innov_var, _H, _K);
    return applyFusion(innov, innov_var, gate_threshold);
}

bool EKF::applyFusion(float innov, float innov_var, float gate_threshold)
{
    if (shouldFuseMeasurement(innov, innov_var, gate_threshold))
    {
        updateErrorState(innov);
        updateCovariancePostFusion();
        injectErrorState();
        resetErrorState();

        return true;
    }
    else
    {
        return false;
    }
}

void EKF::updateCovariancePostFusion()
{
    for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
    {
        _HP[i] = 0.0f;
    }

    for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
    {
        if (_H[i] != 0.0f)
        {
            for (size_t j = 0; j < EKF_NUM_ERROR_STATES; j++)
            {
                _HP[j] += _H[i] * P_current(i, j);
            }
        }
    }

    for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
    {
        if (_K[i] != 0.0f)
        {
            for (size_t j = i; j < EKF_NUM_ERROR_STATES; j++)
            {
                P_current(i, j) -= _K[i] * _HP[j];
            }
        }
    }
}

bool EKF::shouldFuseMeasurement(float innov, float innov_var, float gate_threshold)
{
    // Chi-square gate test for 1 degree of freedom: innov^2 < gate_threshold^2 * innov_var
    return (innov * innov) < (gate_threshold * gate_threshold * innov_var);
}

void EKF::injectErrorState()
{
    quat_t delta_q = {
        .w = 1.0f,
        .x = 0.5f * m_ErrorState.theta.x,
        .y = 0.5f * m_ErrorState.theta.y,
        .z = 0.5f * m_ErrorState.theta.z,
    };
    m_NominalState.attitude = quat_mul(&delta_q, &m_NominalState.attitude);
    quat_normalize(&m_NominalState.attitude);

    m_NominalState.pos.x += m_ErrorState.pos.x;
    m_NominalState.pos.y += m_ErrorState.pos.y;
    m_NominalState.pos.z += m_ErrorState.pos.z;
    m_NominalState.vel.x += m_ErrorState.vel.x;
    m_NominalState.vel.y += m_ErrorState.vel.y;
    m_NominalState.vel.z += m_ErrorState.vel.z;
    m_NominalState.bias_gyro.x += m_ErrorState.bias_gyro.x;
    m_NominalState.bias_gyro.y += m_ErrorState.bias_gyro.y;
    m_NominalState.bias_gyro.z += m_ErrorState.bias_gyro.z;
    m_NominalState.bias_acc.x += m_ErrorState.bias_acc.x;
    m_NominalState.bias_acc.y += m_ErrorState.bias_acc.y;
    m_NominalState.bias_acc.z += m_ErrorState.bias_acc.z;
}

void EKF::resetErrorState()
{
    m_ErrorState.theta = {0, 0, 0};
    m_ErrorState.pos = {0, 0, 0};
    m_ErrorState.vel = {0, 0, 0};
    m_ErrorState.bias_gyro = {0, 0, 0};
    m_ErrorState.bias_acc = {0, 0, 0};
}

void EKF::updateErrorState(float innov)
{
    for (size_t i = 0; i < EKF_NUM_ERROR_STATES; i++)
    {
        m_ErrorState.asArray()[i] += _K[i] * innov;
    }
}