#include "EKFModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/geo/physical_constants.h>
#include <lib/debug/sys_assert.h>
#include <osal/systime.h>
#include <cmath>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define _var(std) ((std) * (std))

void EKFModule::init()
{
    m_EKF.init();

    initCovariance();
}

void EKFModule::run()
{
    if (m_IMUSubscriber.poll())
    {
        processIMU(m_IMUSubscriber.get());
    }
    if (m_GPSSubscriber.poll())
    {
        processGPS(m_GPSSubscriber.get());
    }
    if (m_BaroSubscriber.poll())
    {
        processBaro(m_BaroSubscriber.get());
    }
}

void EKFModule::processIMU(const PubSub::Topics::SensorsIMU &imuData)
{
    uint32_t currentTime = osal_systime_get_ms();

    m_AccelAccum.x += imuData.acc.x * imuData.dt;
    m_AccelAccum.y += imuData.acc.y * imuData.dt;
    m_AccelAccum.z += imuData.acc.z * imuData.dt;
    m_GyroAccum.x += imuData.gyro.x * imuData.dt;
    m_GyroAccum.y += imuData.gyro.y * imuData.dt;
    m_GyroAccum.z += imuData.gyro.z * imuData.dt;
    m_IMUClippingFlagsAccum |= imuData.clippingFlags;
    m_IMUDtAccum += imuData.dt;
    m_IMUSamplesCount++;

    if (!m_EKFInitialized && m_IMUSamplesCount >= EKF_INIT_IMU_SAMPLES)
    {
        if (initState())
        {
            m_EKFInitialized = true;

            LOG_INFO("EKF initialized");
        }

        m_AccelAccum = {0, 0, 0};
        m_GyroAccum = {0, 0, 0};
        m_IMUClippingFlagsAccum = 0;
        m_IMUDtAccum = 0.0f;
        m_IMUSamplesCount = 0;
    }
    else if (m_EKFInitialized && m_IMUDtAccum >= 1.0f / EKF_RATE_HZ)
    {
        EKFIMUData sample;
        sample.delta_velocity = m_AccelAccum;
        sample.delta_angle = m_GyroAccum;
        sample.varAcc = (m_IMUClippingFlagsAccum & PubSub::Helpers::ACC_CLIP_X || m_IMUClippingFlagsAccum & PubSub::Helpers::ACC_CLIP_Y || m_IMUClippingFlagsAccum & PubSub::Helpers::ACC_CLIP_Z) ? _var(EKF_NOISE_ACC_CLIPPING) : _var(EKF_NOISE_ACC);
        sample.varGyro = (m_IMUClippingFlagsAccum & PubSub::Helpers::GYRO_CLIP_X || m_IMUClippingFlagsAccum & PubSub::Helpers::GYRO_CLIP_Y || m_IMUClippingFlagsAccum & PubSub::Helpers::GYRO_CLIP_Z) ? _var(EKF_NOISE_GYRO_CLIPPING) : _var(EKF_NOISE_GYRO);
        sample.dt = m_IMUDtAccum;
        m_IMUBuffer.push(sample, currentTime);

        m_AccelAccum = {0, 0, 0};
        m_GyroAccum = {0, 0, 0};
        m_IMUClippingFlagsAccum = 0;
        m_IMUDtAccum = 0.0f;
        m_IMUSamplesCount = 0;
        m_LastUpdateTime = currentTime;

        outputPredictorForward(sample, currentTime);

        if (!m_EKFEnabled && !m_IMUBuffer.empty() && currentTime - m_IMUBuffer.peekTimestamp() >= EKF_DELAY_HORIZON_MS)
        {
            m_EKFEnabled = true;

            LOG_INFO("EKF enabled");
        }
        if (m_EKFEnabled)
        {
            updateEKF();

            outputPredictorCalculateCorrection(sample.dt);
        }
    }
}

void EKFModule::processGPS(const PubSub::Topics::SensorsGPS &gpsData)
{
    if (!gpsData.gpsIs3dFix || gpsData.pos.lat == 0 || gpsData.pos.lon == 0)
    {
        return;
    }

    if (!m_GPSOriginSet)
    {
        equirect_projection_init(&m_Projection, &gpsData.pos);
        m_GPSOriginSet = true;

        LOG_INFO("GPS origin set to lat: %.6f, lon: %.6f, alt: %.2f", gpsData.pos.lat, gpsData.pos.lon, gpsData.pos.alt);
    }

    if (m_EKFEnabled)
    {
        EKFGPSPosMeasurement posMeas;
        posMeas.pos = equirect_project_to_ned(&m_Projection, &gpsData.pos);
        posMeas.var_hor = _var(gpsData.stddev_horizontal);
        posMeas.var_ver = _var(gpsData.stddev_vertical);
        m_GPSPosBuffer.push(posMeas, osal_systime_get_ms() - EKF_GPS_DELAY_MS);

        if (vec3_mag_compare(&gpsData.vel, EKF_GPS_FUSION_VELOCITY_THRESHOLD) > 0)
        {
            EKFGPSVelMeasurement velMeas;
            velMeas.vel = gpsData.vel;
            velMeas.var = _var(gpsData.stddev_speed);
            m_GPSVelBuffer.push(velMeas, osal_systime_get_ms() - EKF_GPS_DELAY_MS);
        }
        else
        {
            LOG_WARN("GPS velocity below fusion threshold, skipping velocity fusion");
        }
    }
}

void EKFModule::processBaro(const PubSub::Topics::SensorsBaro &baroData)
{
    if (baroData.baroHeight == 0)
    {
        return;
    }

    if (!m_BaroOffsetSet)
    {
        m_BaroOffset = baroData.baroHeight;
        m_BaroOffsetSet = true;

        LOG_INFO("Barometer height offset set to %.2f m", m_BaroOffset);
    }

    if (m_EKFEnabled)
    {
        EKFBaroMeasurement baroMeas;
        baroMeas.height = -(baroData.baroHeight - m_BaroOffset); // Negative because baro height is typically positive upwards, while NED z is positive downwards
        baroMeas.var = _var(EKF_NOISE_BARO);
        m_BaroBuffer.push(baroMeas, osal_systime_get_ms() - EKF_BARO_DELAY_MS);
    }
}

void EKFModule::outputPredictorForward(const EKFIMUData &sample, uint32_t currentTime)
{
    outputPredictorCalculateState(sample);

    m_OutputPredictorBuffer.push(m_CurrentOPState, currentTime);

    m_EKFPublisher.publish({
        .orientation = m_CurrentOPState.attitude,
        .position = m_CurrentOPState.pos,
        .velocity = m_CurrentOPState.vel,
    });
}

void EKFModule::outputPredictorCalculateState(const EKFIMUData &sample)
{
    const float &dt = sample.dt;

    // Attitude prediction (with correction)
    float attFactor = dt / EKF_OUTPUT_PREDICTOR_ATT_TAU;

    m_AttitudeCorrection.x *= attFactor;
    m_AttitudeCorrection.y *= attFactor;
    m_AttitudeCorrection.z *= attFactor;

    quat_t dq = {
        .w = 1.0f,
        .x = 0.5f * (sample.delta_angle.x + m_AttitudeCorrection.x - m_EKF.getState().bias_gyro.x * dt),
        .y = 0.5f * (sample.delta_angle.y + m_AttitudeCorrection.y - m_EKF.getState().bias_gyro.y * dt),
        .z = 0.5f * (sample.delta_angle.z + m_AttitudeCorrection.z - m_EKF.getState().bias_gyro.z * dt),
    };
    m_CurrentOPState.attitude = quat_mul(&m_CurrentOPState.attitude, &dq);
    quat_normalize(&m_CurrentOPState.attitude);

    // Velocity prediction
    vec3_t dv = {
        .x = sample.delta_velocity.x - m_EKF.getState().bias_acc.x * dt,
        .y = sample.delta_velocity.y - m_EKF.getState().bias_acc.y * dt,
        .z = sample.delta_velocity.z - m_EKF.getState().bias_acc.z * dt,
    };
    quat_rotate_vec(&dv, &m_CurrentOPState.attitude);

    vec3_t oldVel = m_CurrentOPState.vel;

    m_CurrentOPState.vel.x += dv.x;
    m_CurrentOPState.vel.y += dv.y;
    m_CurrentOPState.vel.z += dv.z + EARTH_GRAVITY * dt;

    // Position prediction (trapezoidal integration)
    m_CurrentOPState.pos.x += (oldVel.x + m_CurrentOPState.vel.x) * dt * 0.5f;
    m_CurrentOPState.pos.y += (oldVel.y + m_CurrentOPState.vel.y) * dt * 0.5f;
    m_CurrentOPState.pos.z += (oldVel.z + m_CurrentOPState.vel.z) * dt * 0.5f;
}

void EKFModule::outputPredictorCalculateCorrection(float dt)
{
    const EKFNominalState &opState = m_OutputPredictorBuffer.pop();
    const EKFNominalState &ekfState = m_EKF.getState();

    quat_t q_op_inv = quat_conj(&opState.attitude);
    quat_t q_err = quat_mul(&q_op_inv, &ekfState.attitude);
    float sign = (q_err.w >= 0) ? 1.0f : -1.0f;

    m_AttitudeCorrection.x = 2 * sign * q_err.x;
    m_AttitudeCorrection.y = 2 * sign * q_err.y;
    m_AttitudeCorrection.z = 2 * sign * q_err.z;

    float posFactor = dt / EKF_OUTPUT_PREDICTOR_POS_TAU;

    m_PosCorrection.x = ekfState.pos.x - opState.pos.x;
    m_PosCorrection.y = ekfState.pos.y - opState.pos.y;
    m_PosCorrection.z = ekfState.pos.z - opState.pos.z;

    m_PosCorrectionIntegral.x = m_PosCorrection.x + m_PosCorrectionIntegral.x * (1.0f - posFactor);
    m_PosCorrectionIntegral.y = m_PosCorrection.y + m_PosCorrectionIntegral.y * (1.0f - posFactor);
    m_PosCorrectionIntegral.z = m_PosCorrection.z + m_PosCorrectionIntegral.z * (1.0f - posFactor);

    vec3_t currentPosCorrection = {
        .x = m_PosCorrection.x * posFactor + m_PosCorrectionIntegral.x * posFactor,
        .y = m_PosCorrection.y * posFactor + m_PosCorrectionIntegral.y * posFactor,
        .z = m_PosCorrection.z * posFactor + m_PosCorrectionIntegral.z * posFactor,
    };

    float velFactor = dt / EKF_OUTPUT_PREDICTOR_VEL_TAU;

    m_VelCorrection.x = ekfState.vel.x - opState.vel.x;
    m_VelCorrection.y = ekfState.vel.y - opState.vel.y;
    m_VelCorrection.z = ekfState.vel.z - opState.vel.z;

    m_VelCorrectionIntegral.x = m_VelCorrection.x + m_VelCorrectionIntegral.x * (1.0f - velFactor);
    m_VelCorrectionIntegral.y = m_VelCorrection.y + m_VelCorrectionIntegral.y * (1.0f - velFactor);
    m_VelCorrectionIntegral.z = m_VelCorrection.z + m_VelCorrectionIntegral.z * (1.0f - velFactor);

    vec3_t currentVelCorrection = {
        .x = m_VelCorrection.x * velFactor + m_VelCorrectionIntegral.x * velFactor,
        .y = m_VelCorrection.y * velFactor + m_VelCorrectionIntegral.y * velFactor,
        .z = m_VelCorrection.z * velFactor + m_VelCorrectionIntegral.z * velFactor,
    };

    // Apply corrections to output predictor buffer and current OP state
    for (size_t i = 0; i < m_OutputPredictorBuffer.size(); i++)
    {
        m_OutputPredictorBuffer.get(i).vel.x += currentVelCorrection.x;
        m_OutputPredictorBuffer.get(i).vel.y += currentVelCorrection.y;
        m_OutputPredictorBuffer.get(i).vel.z += currentVelCorrection.z;
        m_OutputPredictorBuffer.get(i).pos.x += currentPosCorrection.x;
        m_OutputPredictorBuffer.get(i).pos.y += currentPosCorrection.y;
        m_OutputPredictorBuffer.get(i).pos.z += currentPosCorrection.z;
    }

    // Also apply to current OP state to prevent discontinuities
    m_CurrentOPState = m_OutputPredictorBuffer.getNewest();
}

uint32_t EKFModule::getMinMeasurementTimestamp() const
{
    uint32_t minTimestamp = UINT32_MAX;

    if (!m_GPSPosBuffer.empty())
    {
        minTimestamp = MIN(minTimestamp, m_GPSPosBuffer.peekTimestamp());
    }
    if (!m_GPSVelBuffer.empty())
    {
        minTimestamp = MIN(minTimestamp, m_GPSVelBuffer.peekTimestamp());
    }
    if (!m_BaroBuffer.empty())
    {
        minTimestamp = MIN(minTimestamp, m_BaroBuffer.peekTimestamp());
    }

    return minTimestamp;
}

void EKFModule::updateEKF()
{
    if (m_IMUBuffer.size() < 2)
    {
        LOG_ERROR("Not enough IMU data to update EKF");

        return;
    }

    const EKFIMUData &imuData = m_IMUBuffer.pop();

    m_EKF.predictState(imuData);
    m_EKF.predictCovariance(imuData);

    addBiasNoiseToCovariance(imuData.dt);

    uint32_t ekfTime = m_IMUBuffer.peekTimestamp();
    size_t measCount = 0;

    for (measCount = 0; measCount < EKF_MAX_FUSIONS_PER_UPDATE; measCount++)
    {
        uint32_t minMeasTimestamp = getMinMeasurementTimestamp();

        if (minMeasTimestamp > ekfTime)
        {
            break;
        }

        bool cleanFusion = false;

        if (!m_GPSPosBuffer.empty() && m_GPSPosBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            cleanFusion = m_EKF.fuseGPSPosition(m_GPSPosBuffer.pop(), EKF_GATE_THRESHOLD_GPS_POS);
        }
        else if (!m_GPSVelBuffer.empty() && m_GPSVelBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            cleanFusion = m_EKF.fuseGPSVelocity(m_GPSVelBuffer.pop(), EKF_GATE_THRESHOLD_GPS_VEL);
        }
        else if (!m_BaroBuffer.empty() && m_BaroBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            cleanFusion = m_EKF.fuseBaroHeight(m_BaroBuffer.pop(), EKF_GATE_THRESHOLD_BARO);
        }
        else
        {
            break;
        }

        if (!cleanFusion)
        {
            LOG_WARN("Measurement fusion failed due to gate check, skipping remaining fusions for this update");
        }
    }

    SYS_ASSERT(measCount < EKF_MAX_FUSIONS_PER_UPDATE);
}

bool EKFModule::initState()
{
    vec3_t g_vec_norm = {
        .x = 0,
        .y = 0,
        .z = -1,
    };

    vec3_t avgAcc = {
        .x = m_AccelAccum.x / m_IMUDtAccum,
        .y = m_AccelAccum.y / m_IMUDtAccum,
        .z = m_AccelAccum.z / m_IMUDtAccum,
    };

    float acc_len = vec3_mag(&avgAcc);

    if (fabsf(acc_len - EARTH_GRAVITY) > EKF_INIT_ACC_MAG_ERROR_THRESHOLD)
    {
        LOG_ERROR("Average acceleration magnitude is too low for reliable attitude initialization");

        return false;
    }

    vec3_normalize(&avgAcc);

    quat_t initialAttitude = quat_from_vecs(&avgAcc, &g_vec_norm);

    m_CurrentOPState.attitude = initialAttitude;
    m_CurrentOPState.pos = {0, 0, 0};
    m_CurrentOPState.vel = {0, 0, 0};
    m_CurrentOPState.bias_acc = {0, 0, 0};
    m_CurrentOPState.bias_gyro = {0, 0, 0};

    m_EKF.getState() = m_CurrentOPState;

    return true;
}

void EKFModule::initCovariance()
{
    m_EKF.getCovarianceElement(0, 0) = EKF_COV_DEFAULT_ATT_ROLL_PITCH;
    m_EKF.getCovarianceElement(1, 1) = EKF_COV_DEFAULT_ATT_ROLL_PITCH;
    m_EKF.getCovarianceElement(2, 2) = EKF_COV_DEFAULT_ATT_YAW;

    for (size_t i = 3; i < 6; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_POS;
    }

    for (size_t i = 6; i < 9; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_VEL;
    }

    for (size_t i = 9; i < 12; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_B_GYRO;
    }

    for (size_t i = 12; i < 15; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_B_ACC;
    }
}

void EKFModule::addBiasNoiseToCovariance(float dt)
{
    for (size_t i = 9; i < 12; i++)
    {
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_GYRO_BIAS) * dt * dt;
    }

    for (size_t i = 12; i < 15; i++)
    {
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_ACC_BIAS) * dt * dt;
    }
}