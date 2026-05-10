#include "EKFModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/geo/physical_constants.h>
#include <lib/geo/geo_mag.h>
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
    if (m_MagSubscriber.poll())
    {
        processMag(m_MagSubscriber.get());
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
            velMeas.var_hor = _var(gpsData.stddev_speed);
            velMeas.var_ver = _var(gpsData.stddev_speed * EKF_GPS_VEL_D_NOISE_SCALE);
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

void EKFModule::processMag(const PubSub::Topics::SensorsMag &magData)
{
#if EKF_USE_MAG
    if (m_EKFEnabled)
    {
        EKFMagMeasurement magMeas;
        magMeas.mag = magData.mag;
        magMeas.var = _var(EKF_NOISE_MAG);
        m_MagBuffer.push(magMeas, osal_systime_get_ms() - EKF_MAG_DELAY_MS);
    }
    else
    {
        m_MagAccum.x += magData.mag.x;
        m_MagAccum.y += magData.mag.y;
        m_MagAccum.z += magData.mag.z;
        m_MagSamplesCount++;
    }
#else
    (void)magData;
#endif
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

    // Attitude prediction (with complementary filter correction)
    float attFactor = 0.5f * dt / (float)EKF_DELAY_HORIZON_MS * 1000.0f;

    // Apply fraction of stored correction this step, then decay the residual
    vec3_t attCorrThisStep = {
        .x = m_AttitudeCorrection.x * attFactor,
        .y = m_AttitudeCorrection.y * attFactor,
        .z = m_AttitudeCorrection.z * attFactor,
    };
    m_AttitudeCorrection.x -= attCorrThisStep.x;
    m_AttitudeCorrection.y -= attCorrThisStep.y;
    m_AttitudeCorrection.z -= attCorrThisStep.z;

    // Gyro integration: body-frame rotation, applied from the right.
    quat_t dq = {
        .w = 1.0f,
        .x = 0.5f * (sample.delta_angle.x - m_EKF.getState().bias_gyro.x * dt),
        .y = 0.5f * (sample.delta_angle.y - m_EKF.getState().bias_gyro.y * dt),
        .z = 0.5f * (sample.delta_angle.z - m_EKF.getState().bias_gyro.z * dt),
    };
    m_CurrentOPState.attitude = quat_mul(&m_CurrentOPState.attitude, &dq);

    // Attitude correction: global-frame rotation, applied from the left.
    quat_t dq_corr = {
        .w = 1.0f,
        .x = 0.5f * attCorrThisStep.x,
        .y = 0.5f * attCorrThisStep.y,
        .z = 0.5f * attCorrThisStep.z,
    };
    m_CurrentOPState.attitude = quat_mul(&dq_corr, &m_CurrentOPState.attitude);
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
    // Calculate error in global frame. If we calculated error in body frame, then as the body rotates the correction would become less and less valid, especially at high angular rates. By calculating in global frame, the correction remains valid regardless of body rotation.
    // q_ekf = q_err * q_op
    quat_t q_err = quat_mul(&ekfState.attitude, &q_op_inv);
    float sign = (q_err.w >= 0) ? 1.0f : -1.0f;

    m_AttitudeCorrection.x = 2.0f * sign * q_err.x;
    m_AttitudeCorrection.y = 2.0f * sign * q_err.y;
    m_AttitudeCorrection.z = 2.0f * sign * q_err.z;

    float posAlpha = dt / EKF_OUTPUT_PREDICTOR_POS_TAU;
    float velAlpha = dt / EKF_OUTPUT_PREDICTOR_VEL_TAU;

    vec3_t posCorr = {
        .x = (ekfState.pos.x - opState.pos.x) * posAlpha,
        .y = (ekfState.pos.y - opState.pos.y) * posAlpha,
        .z = (ekfState.pos.z - opState.pos.z) * posAlpha,
    };
    vec3_t velCorr = {
        .x = (ekfState.vel.x - opState.vel.x) * velAlpha,
        .y = (ekfState.vel.y - opState.vel.y) * velAlpha,
        .z = (ekfState.vel.z - opState.vel.z) * velAlpha,
    };

    for (size_t i = 0; i < m_OutputPredictorBuffer.size(); i++)
    {
        m_OutputPredictorBuffer.get(i).pos.x += posCorr.x;
        m_OutputPredictorBuffer.get(i).pos.y += posCorr.y;
        m_OutputPredictorBuffer.get(i).pos.z += posCorr.z;
        m_OutputPredictorBuffer.get(i).vel.x += velCorr.x;
        m_OutputPredictorBuffer.get(i).vel.y += velCorr.y;
        m_OutputPredictorBuffer.get(i).vel.z += velCorr.z;
    }

    m_CurrentOPState = m_OutputPredictorBuffer.getNewest();

    // Copy static fields
    m_CurrentOPState.mag = ekfState.mag;
    m_CurrentOPState.bias_acc = ekfState.bias_acc;
    m_CurrentOPState.bias_gyro = ekfState.bias_gyro;
    m_CurrentOPState.bias_mag = ekfState.bias_mag;
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
    if (!m_MagBuffer.empty())
    {
        minTimestamp = MIN(minTimestamp, m_MagBuffer.peekTimestamp());
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
        else if (!m_MagBuffer.empty() && m_MagBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            cleanFusion = m_EKF.fuseMag(m_MagBuffer.pop(), EKF_GATE_THRESHOLD_MAG);
        }
        else
        {
            break;
        }

        if (!cleanFusion)
        {
            LOG_WARN("Measurement fusion failed gate check");
        }
    }

    if (measCount >= EKF_MAX_FUSIONS_PER_UPDATE)
    {
        LOG_ERROR("EKF hit max fusions per update - measurement backlog detected");
    }
}

bool EKFModule::initState()
{
    vec3_t avgAcc = {
        .x = m_AccelAccum.x / m_IMUDtAccum,
        .y = m_AccelAccum.y / m_IMUDtAccum,
        .z = m_AccelAccum.z / m_IMUDtAccum,
    };

    float acc_len = vec3_mag(&avgAcc);

    if (fabsf(acc_len - EARTH_GRAVITY) > EKF_INIT_ACC_MAG_ERROR_THRESHOLD)
    {
        LOG_ERROR("Average acceleration (%f) magnitude is invalid for reliable attitude initialization", acc_len);

        return false;
    }

    // TODO: Perform real init sequence
#if EKF_USE_MAG
    // vec3_t avgMag = {
    //     .x = m_MagAccum.x / m_MagSamplesCount,
    //     .y = m_MagAccum.y / m_MagSamplesCount,
    //     .z = m_MagAccum.z / m_MagSamplesCount,
    // };

    // vec3_t avgMag = {0.191f, 0.019f, 0.462f};
    vec3_t avgMag = {1.0f, 0.0f, 0.0f};

    quat_t initialAttitude = quat_from_acc_mag(&avgAcc, &avgMag);

    // vec3_t initialMagField = geo_mag_field_vector({});
    vec3_t initialMagField = {0.191f, 0.019f, 0.462f};

    LOG_DEBUG("Initial mag field: x=%.4f, y=%.4f, z=%.4f", initialMagField.x, initialMagField.y, initialMagField.z);
#else
    vec3_normalize(&avgAcc);

    vec3_t g_vec_norm = {0, 0, -1};
    quat_t initialAttitude = quat_from_vecs(&avgAcc, &g_vec_norm);
    vec3_t initialMagField = {0, 0, 0};
#endif

    LOG_DEBUG("Initial attitude: w=%.4f, x=%.4f, y=%.4f, z=%.4f", initialAttitude.w, initialAttitude.x, initialAttitude.y, initialAttitude.z);

    m_CurrentOPState.attitude = initialAttitude;
    m_CurrentOPState.pos = {0, 0, 0};
    m_CurrentOPState.vel = {0, 0, 0};
    m_CurrentOPState.mag = initialMagField;
    m_CurrentOPState.bias_acc = {0, 0, 0};
    m_CurrentOPState.bias_gyro = {0, 0, 0};
    m_CurrentOPState.bias_mag = {0, 0, 0};

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
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_MAG;
    }

    for (size_t i = 12; i < 15; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_B_GYRO;
    }

    for (size_t i = 15; i < 18; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_B_ACC;
    }

    for (size_t i = 18; i < 21; i++)
    {
        m_EKF.getCovarianceElement(i, i) = EKF_COV_DEFAULT_B_MAG;
    }
}

void EKFModule::addBiasNoiseToCovariance(float dt)
{
    for (size_t i = 12; i < 15; i++)
    {
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_GYRO_BIAS) * dt;
    }

    for (size_t i = 15; i < 18; i++)
    {
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_ACC_BIAS) * dt;
    }

    for (size_t i = 18; i < 21; i++)
    {
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_MAG_BIAS) * dt;
    }
}