#include "EKFModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/maths/math_constants.h>
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
            initCovariance();

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
        LOG_ERROR("Invalid GPS fix, skipping");

        if (!m_GPSInitialized)
        {
            m_GPSNStats.reset();
            m_GPSEStats.reset();
            m_GPSDStats.reset();
        }

        return;
    }

    if (!m_GPSInitialized)
    {
        if (!m_GPSOriginSet)
        {
            equirect_projection_init(&m_Projection, &gpsData.pos);

            m_GPSOriginSet = true;

            LOG_INFO("Temporary GPS origin set to lat=%.6f, lon=%.6f, alt=%.2f m", m_Projection.ref.lat, m_Projection.ref.lon, m_Projection.ref.alt);
        }

        vec3_t proj_pos = equirect_project_to_ned(&m_Projection, &gpsData.pos);

        m_GPSNStats.push(proj_pos.x);
        m_GPSEStats.push(proj_pos.y);
        m_GPSDStats.push(proj_pos.z);

        if (m_GPSNStats.count() >= EKF_INIT_GPS_SAMPLES && m_GPSEStats.count() >= EKF_INIT_GPS_SAMPLES && m_GPSDStats.count() >= EKF_INIT_GPS_SAMPLES)
        {
            if (m_GPSNStats.stddev() < EKF_NOISE_GPS_POS_HOR_INIT && m_GPSEStats.stddev() < EKF_NOISE_GPS_POS_HOR_INIT && m_GPSDStats.stddev() < EKF_NOISE_GPS_POS_VER_INIT)
            {
                LOG_INFO("GPS origin lat/lon/alt stddev: %.6f / %.6f / %.2f m - origin will be set", m_GPSNStats.stddev(), m_GPSEStats.stddev(), m_GPSDStats.stddev());

                m_GPSInitialized = true;
            }
            else
            {
                LOG_ERROR("GPS origin lat/lon/alt readings too noisy for reliable origin initialization (stddev: %.6f / %.6f / %.2f m), retrying origin initialization", m_GPSNStats.stddev(), m_GPSEStats.stddev(), m_GPSDStats.stddev());

                m_GPSOriginSet = false;
            }

            m_GPSNStats.reset();
            m_GPSEStats.reset();
            m_GPSDStats.reset();
        }

        return;
    }

    if (m_EKFEnabled)
    {
        EKFGPSPosMeasurement posMeas;
        posMeas.pos = equirect_project_to_ned(&m_Projection, &gpsData.pos);
        posMeas.var_hor = _var(gpsData.stddev_horizontal);
        posMeas.var_ver = _var(gpsData.stddev_vertical);
        m_GPSPosBuffer.push(posMeas, osal_systime_get_ms() - EKF_DELAY_MS_GPS);

        if (vec3_mag_compare(&gpsData.vel, EKF_GPS_FUSION_VELOCITY_THRESHOLD) > 0)
        {
            EKFGPSVelMeasurement velMeas;
            velMeas.vel = gpsData.vel;
            velMeas.var_hor = _var(gpsData.stddev_speed);
            velMeas.var_ver = _var(gpsData.stddev_speed * EKF_GPS_VEL_D_NOISE_SCALE);
            m_GPSVelBuffer.push(velMeas, osal_systime_get_ms() - EKF_DELAY_MS_GPS);
        }
    }
}

void EKFModule::processBaro(const PubSub::Topics::SensorsBaro &baroData)
{
    if (baroData.baroHeight == 0)
    {
        LOG_ERROR("Invalid barometer height reading, skipping");

        if (!m_BaroOffsetSet)
        {
            m_BaroStats.reset();
        }

        return;
    }

    if (!m_BaroOffsetSet)
    {
        m_BaroStats.push(baroData.baroHeight);

        if (m_BaroStats.count() >= EKF_INIT_BARO_SAMPLES)
        {
            if (m_BaroStats.stddev() < EKF_NOISE_BARO_INIT)
            {
                m_BaroOffset = m_BaroStats.getMean();
                m_BaroOffsetSet = true;

                LOG_INFO("Barometer height offset set to %.2f m", m_BaroOffset);
            }
            else
            {
                LOG_ERROR("Barometer height readings too noisy for reliable offset initialization (stddev: %.2f m), retrying offset initialization", m_BaroStats.stddev());
            }

            m_BaroStats.reset();
        }

        return;
    }

    if (m_EKFEnabled)
    {
        EKFBaroMeasurement baroMeas;
        baroMeas.height = -(baroData.baroHeight - m_BaroOffset); // Negative because baro height is typically positive upwards, while NED z is positive downwards
        baroMeas.var = _var(EKF_NOISE_BARO);
        m_BaroBuffer.push(baroMeas, osal_systime_get_ms() - EKF_DELAY_MS_BARO);
    }
}

void EKFModule::processMag(const PubSub::Topics::SensorsMag &magData)
{
#if EKF_USE_MAG
    if (!m_GPSOriginSet)
    {
        return;
    }

    if (!m_MagInitDone)
    {
        m_MagXStats.push(magData.mag.x);
        m_MagYStats.push(magData.mag.y);
        m_MagZStats.push(magData.mag.z);

        if (m_MagXStats.count() >= EKF_INIT_MAG_SAMPLES && m_MagYStats.count() >= EKF_INIT_MAG_SAMPLES && m_MagZStats.count() >= EKF_INIT_MAG_SAMPLES)
        {
            float strength = sqrtf(m_MagXStats.getMean() * m_MagXStats.getMean() + m_MagYStats.getMean() * m_MagYStats.getMean() + m_MagZStats.getMean() * m_MagZStats.getMean());
            float expectedStrength = geo_mag_get_strength(&m_Projection.ref);

            if (fabsf(strength - expectedStrength) < EKF_INIT_MAG_STRENGTH_EPS && m_MagXStats.stddev() < EKF_NOISE_MAG_INIT && m_MagYStats.stddev() < EKF_NOISE_MAG_INIT && m_MagZStats.stddev() < EKF_NOISE_MAG_INIT)
            {
                // Get expected magnetic field vector at current location, to be used as reference for yaw initialization and fusion
                vec3_t expectedMagNED = geo_mag_field_vector(&m_Projection.ref);
                m_CurrentOPState.mag = expectedMagNED;

                LOG_DEBUG("Expected magnetic field vector at current location (NED): x=%.4f, y=%.4f, z=%.4f", expectedMagNED.x, expectedMagNED.y, expectedMagNED.z);

                // Normalize horizontal component of expected magnetic field to get reference direction for yaw
                expectedMagNED.z = 0;
                vec3_normalize(&expectedMagNED);

                vec3_t magNED = {
                    .x = m_MagXStats.getMean(),
                    .y = m_MagYStats.getMean(),
                    .z = m_MagZStats.getMean(),
                };

                LOG_DEBUG("Magnetometer readings (NED): x=%.4f, y=%.4f, z=%.4f", magNED.x, magNED.y, magNED.z);

                // Rotate measured mag and normalize horizontal component to get measured direction for yaw
                quat_rotate_vec(&magNED, &m_CurrentOPState.attitude);

                magNED.z = 0;
                vec3_normalize(&magNED);

                // Apply to attitude
                quat_t dq = quat_from_vecs(&magNED, &expectedMagNED);
                LOG_DEBUG("Delta quaternion for mag: w=%.4f, x=%.4f, y=%.4f, z=%.4f", dq.w, dq.x, dq.y, dq.z);

                m_CurrentOPState.attitude = quat_mul(&dq, &m_CurrentOPState.attitude);
                quat_normalize(&m_CurrentOPState.attitude);

                LOG_INFO("Initialized attitude (with MAG): w=%.4f, x=%.4f, y=%.4f, z=%.4f", m_CurrentOPState.attitude.w, m_CurrentOPState.attitude.x, m_CurrentOPState.attitude.y, m_CurrentOPState.attitude.z);

                yawReset();

                m_MagInitDone = true;

                LOG_INFO("Magnetometer initialized");
            }
            else
            {
                LOG_ERROR("Magnetometer readings too noisy (stddev: x=%.5f, y=%.5f, z=%.5f) or incorrect strength (expected: %.3f, got: %.3f), retrying magnetometer initialization", m_MagXStats.stddev(), m_MagYStats.stddev(), m_MagZStats.stddev(), expectedStrength, strength);
            }

            m_MagXStats.reset();
            m_MagYStats.reset();
            m_MagZStats.reset();
        }

        return;
    }

    if (m_EKFEnabled)
    {
        EKFMagMeasurement magMeas;
        magMeas.mag = magData.mag;
        magMeas.var = _var(EKF_NOISE_MAG);
        m_MagBuffer.push(magMeas, osal_systime_get_ms() - EKF_DELAY_MS_MAG);
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
    // Equation: q_ekf = q_err * q_op
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

        if (!m_GPSPosBuffer.empty() && m_GPSPosBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            if (!m_EKF.fuseGPSPosition(m_GPSPosBuffer.pop(), EKF_GATE_THRESHOLD_GPS_POS))
            {
                LOG_WARN("GPS position fusion failed gate check");
            }
        }
        else if (!m_GPSVelBuffer.empty() && m_GPSVelBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            if (!m_EKF.fuseGPSVelocity(m_GPSVelBuffer.pop(), EKF_GATE_THRESHOLD_GPS_VEL))
            {
                LOG_WARN("GPS velocity fusion failed gate check");
            }
        }
        else if (!m_BaroBuffer.empty() && m_BaroBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            if (!m_EKF.fuseBaroHeight(m_BaroBuffer.pop(), EKF_GATE_THRESHOLD_BARO))
            {
                LOG_WARN("Baro height fusion failed gate check");
            }
        }
        else if (!m_MagBuffer.empty() && m_MagBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            if (!m_EKF.fuseMag(m_MagBuffer.pop(), EKF_GATE_THRESHOLD_MAG))
            {
                LOG_WARN("Mag fusion failed gate check");
            }
        }
        else
        {
            break;
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

    if (fabsf(acc_len - EARTH_GRAVITY) > EKF_INIT_IMU_EPS)
    {
        LOG_ERROR("Average acceleration (%f) magnitude is invalid for reliable attitude initialization", acc_len);

        return false;
    }

    vec3_normalize(&avgAcc);

    vec3_t g_vec_norm = {0, 0, -1};
    quat_t initialAttitude = quat_from_vecs(&avgAcc, &g_vec_norm);

    LOG_INFO("Initial attitude (only IMU): w=%.4f, x=%.4f, y=%.4f, z=%.4f", initialAttitude.w, initialAttitude.x, initialAttitude.y, initialAttitude.z);

    m_CurrentOPState.attitude = initialAttitude;
    m_CurrentOPState.pos = {0, 0, 0};
    m_CurrentOPState.vel = {0, 0, 0};
    m_CurrentOPState.mag = {0, 0, 0};
    m_CurrentOPState.bias_acc = {0, 0, 0};
    m_CurrentOPState.bias_gyro = {0, 0, 0};
    m_CurrentOPState.bias_mag = {0, 0, 0};

    m_EKF.getState() = m_CurrentOPState;

    return true;
}

void EKFModule::initCovariance()
{
    m_EKF.getCovarianceElement(0, 0) = EKF_COV_DEFAULT_ATT_DEFAULT;
    m_EKF.getCovarianceElement(1, 1) = EKF_COV_DEFAULT_ATT_DEFAULT;
    m_EKF.getCovarianceElement(2, 2) = EKF_COV_DEFAULT_ATT_UNKNOWN;

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

void EKFModule::yawReset()
{
    // Perform state reset and buffer repropagation
    for (size_t i = 0; i < m_OutputPredictorBuffer.size(); i++)
    {
        m_OutputPredictorBuffer.get(i).attitude = m_CurrentOPState.attitude;
        m_OutputPredictorBuffer.get(i).mag = m_CurrentOPState.mag;
    }

    m_EKF.getState().attitude = m_CurrentOPState.attitude;
    m_EKF.getState().mag = m_CurrentOPState.mag;

    // Set high initial yaw uncertainty since we don't know the initial yaw
    m_EKF.getCovarianceElement(2, 2) = EKF_COV_DEFAULT_ATT_UNKNOWN;

    LOG_INFO("EKF yaw reset performed");
}