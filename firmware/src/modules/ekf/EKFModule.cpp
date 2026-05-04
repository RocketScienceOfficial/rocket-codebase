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
        initState();
        initCovariance();

        m_AccelAccum = {0, 0, 0};
        m_GyroAccum = {0, 0, 0};
        m_IMUClippingFlagsAccum = 0;
        m_IMUDtAccum = 0.0f;
        m_IMUSamplesCount = 0;

        m_EKFInitialized = true;

        LOG_INFO("EKF initialized");
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

        outputPredictorIntegrate(sample, currentTime);

        if (!m_EKFEnabled && !m_IMUBuffer.empty() && currentTime - m_IMUBuffer.peekTimestamp() >= EKF_DELAY_HORIZON_MS)
        {
            m_EKFEnabled = true;

            LOG_INFO("EKF enabled");
        }
        if (m_EKFEnabled)
        {
            updateEKF();
        }

        outputPredictorCalculateCorrection();
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
    }
}

void EKFModule::processBaro(const PubSub::Topics::SensorsBaro &baroData)
{
    if (baroData.baroHeight == 0)
    {
        return;
    }

    if (m_BaroOffsetSet)
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

void EKFModule::outputPredictorIntegrate(const EKFIMUData &sample, uint32_t currentTime)
{
    EKF::predictExplicitState(m_CurrentOPState, m_OPNextCorrection, sample);

    m_OutputPredictorBuffer.push(m_CurrentOPState, currentTime);

    outputPredictorPublishState();
}

void EKFModule::outputPredictorPublishState()
{
    m_EKFPublisher.publish({
        .orientation = m_CurrentOPState.attitude,
        .position = m_CurrentOPState.pos,
        .velocity = m_CurrentOPState.vel,
    });
}

void EKFModule::outputPredictorCalculateCorrection()
{
    const EKFNominalState &opState = m_OutputPredictorBuffer.pop();
    const EKFNominalState &ekfState = m_EKF.getState();

    quat_t q_op_inv = quat_conj(&opState.attitude);
    quat_t q_err = quat_mul(&q_op_inv, &ekfState.attitude);
    float sign = (q_err.w >= 0) ? 1.0f : -1.0f;

    m_OPNextCorrection.theta = {
        .x = 2 * sign * q_err.x / EKF_OUTPUT_PREDICTOR_ATT_TAU,
        .y = 2 * sign * q_err.y / EKF_OUTPUT_PREDICTOR_ATT_TAU,
        .z = 2 * sign * q_err.z / EKF_OUTPUT_PREDICTOR_ATT_TAU,
    };
    m_OPNextCorrection.pos = {
        .x = (ekfState.pos.x - opState.pos.x) / EKF_OUTPUT_PREDICTOR_POS_TAU,
        .y = (ekfState.pos.y - opState.pos.y) / EKF_OUTPUT_PREDICTOR_POS_TAU,
        .z = (ekfState.pos.z - opState.pos.z) / EKF_OUTPUT_PREDICTOR_POS_TAU,
    };
    m_OPNextCorrection.vel = {
        .x = (ekfState.vel.x - opState.vel.x) / EKF_OUTPUT_PREDICTOR_VEL_TAU,
        .y = (ekfState.vel.y - opState.vel.y) / EKF_OUTPUT_PREDICTOR_VEL_TAU,
        .z = (ekfState.vel.z - opState.vel.z) / EKF_OUTPUT_PREDICTOR_VEL_TAU,
    };
    m_OPNextCorrection.bias_acc = {
        .x = (ekfState.bias_acc.x - opState.bias_acc.x) / EKF_OUTPUT_PREDICTOR_B_ACC_TAU,
        .y = (ekfState.bias_acc.y - opState.bias_acc.y) / EKF_OUTPUT_PREDICTOR_B_ACC_TAU,
        .z = (ekfState.bias_acc.z - opState.bias_acc.z) / EKF_OUTPUT_PREDICTOR_B_ACC_TAU,
    };
    m_OPNextCorrection.bias_gyro = {
        .x = (ekfState.bias_gyro.x - opState.bias_gyro.x) / EKF_OUTPUT_PREDICTOR_B_GYRO_TAU,
        .y = (ekfState.bias_gyro.y - opState.bias_gyro.y) / EKF_OUTPUT_PREDICTOR_B_GYRO_TAU,
        .z = (ekfState.bias_gyro.z - opState.bias_gyro.z) / EKF_OUTPUT_PREDICTOR_B_GYRO_TAU,
    };
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

        bool wasFused = false;

        if (!m_GPSPosBuffer.empty() && m_GPSPosBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            wasFused = m_EKF.fuseGPSPosition(m_GPSPosBuffer.pop(), EKF_GATE_THRESHOLD_GPS_POS);
        }
        else if (!m_GPSVelBuffer.empty() && m_GPSVelBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            wasFused = m_EKF.fuseGPSVelocity(m_GPSVelBuffer.pop(), EKF_GATE_THRESHOLD_GPS_VEL);
        }
        else if (!m_BaroBuffer.empty() && m_BaroBuffer.peekTimestamp() <= minMeasTimestamp)
        {
            wasFused = m_EKF.fuseBaroHeight(m_BaroBuffer.pop(), EKF_GATE_THRESHOLD_BARO);
        }
        else
        {
            break;
        }

        if (!wasFused)
        {
            LOG_WARN("Measurement at timestamp %u failed fusion consistency check and was rejected", minMeasTimestamp);
        }
    }

    SYS_ASSERT(measCount < EKF_MAX_FUSIONS_PER_UPDATE);
}

void EKFModule::initState()
{
    vec3_t g_vec_norm = {
        .x = 0,
        .y = 0,
        .z = 1,
    };
    vec3_t avgAcc = {
        .x = m_AccelAccum.x / m_IMUDtAccum,
        .y = m_AccelAccum.y / m_IMUDtAccum,
        .z = m_AccelAccum.z / m_IMUDtAccum,
    };
    vec3_normalize(&avgAcc);

    quat_t initialAttitude = quat_from_vecs(&avgAcc, &g_vec_norm);

    m_EKF.getState().attitude = initialAttitude;
    m_EKF.getState().pos = {0, 0, 0};
    m_EKF.getState().vel = {0, 0, 0};
    m_EKF.getState().bias_acc = {0, 0, 0};
    m_EKF.getState().bias_gyro = {0, 0, 0};
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
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_GYRO_BIAS) * dt;
    }

    for (size_t i = 12; i < 15; i++)
    {
        m_EKF.getCovarianceElement(i, i) += _var(EKF_NOISE_ACC_BIAS) * dt;
    }
}