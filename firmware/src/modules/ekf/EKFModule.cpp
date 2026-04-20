#include "EKFModule.h"
#include "modules/common/ModuleLogger.h"
#include <lib/geo/physical_constants.h>
#include <osal/systime.h>

#define ORIENTATION_MADGWICK_BETA 0.033f
#define ORIENTATION_ACCELERATION_SWITCH_THRESHOLD (2.0f * EARTH_GRAVITY)
#define ORIENTATION_HIGH_GAIN_DISABLE_TIME_MS 3000

#define EKF_ACC_VARIANCE 0.02f
#define EKF_GPS_VARIANCE 1.7f
#define EKF_BARO_VARIANCE 0.25f
#define EKF_OUTDATED_MEASUREMENT_VARIANCE 1000000.0f

void EKFModule::init()
{
    m_EKFVars = {
        .varAcc = EKF_ACC_VARIANCE,
        .varGPS = EKF_GPS_VARIANCE,
        .varBar = EKF_BARO_VARIANCE,
    };
    m_EKF.init();
    m_EKF.setVariances(m_EKFVars);

    m_CurrentFrame.orientation = (quat_t){1.0f, 0.0f, 0.0f, 0.0f};
    m_MadgwickGain = 1.0f;
    m_HighGainDisableTimer = osal_systime_get_ms();
}

void EKFModule::run()
{
    updateGPSData();
    updateBarometerData();
    updateIMUData();
}

void EKFModule::updateGPSData()
{
    if (m_GPSSubscriber.poll())
    {
        const auto &gpsData = m_GPSSubscriber.get();

        if (m_BaseGPSPos.lat == 0 && gpsData.pos.lat != 0)
        {
            m_BaseGPSPos = gpsData.pos;
            
            LOG_INFO("Base GPS position set to lat: %.6f, lon: %.6f, alt: %.2f", m_BaseGPSPos.lat, m_BaseGPSPos.lon, m_BaseGPSPos.alt);
        }
        else
        {
            m_NEDPos = geo_to_ned(gpsData.pos, m_BaseGPSPos);
        }

        int satCount = gpsData.gpsSatellitesCount;
        float var = 0.0f;

        if (satCount <= 4)
            var = EKF_OUTDATED_MEASUREMENT_VARIANCE;
        else if (satCount == 5)
            var = 50.0f;
        else if (satCount == 6)
            var = 20.0f;
        else if (satCount == 7)
            var = 10.0f;
        else if (satCount == 8)
            var = 7.0f;
        else if (satCount == 9)
            var = 5.0f;
        else if (satCount == 10)
            var = 3.0f;
        else if (satCount == 11)
            var = 2.0f;
        else
            var = EKF_GPS_VARIANCE;

        m_EKFVars.varGPS = var;
        m_EKF.setVariances(m_EKFVars);
    }
}

void EKFModule::updateBarometerData()
{
    if (m_BaroSubscriber.poll())
    {
        const auto &baroData = m_BaroSubscriber.get();

        if (m_BaroHeightOffset == 0 && baroData.baroHeight != 0)
        {
            m_BaroHeightOffset = baroData.baroHeight;

            LOG_INFO("Barometer height offset set to %.2f m", m_BaroHeightOffset);
        }
        else
        {
            m_BaroHeight = baroData.baroHeight - m_BaroHeightOffset;
        }

        m_EKFVars.varBar = EKF_BARO_VARIANCE;
        m_EKF.setVariances(m_EKFVars);
    }
}

void EKFModule::updateIMUData()
{
    if (m_IMUSubscriber.poll())
    {
        const auto &imuData = m_IMUSubscriber.get();

        if (m_HighGainDisableTimer != 0 && osal_systime_get_ms() - m_HighGainDisableTimer >= ORIENTATION_HIGH_GAIN_DISABLE_TIME_MS)
        {
            m_HighGainDisableTimer = 0;
            m_MadgwickGain = ORIENTATION_MADGWICK_BETA;
        }

        if (vec3_mag_compare(&imuData.acc, ORIENTATION_ACCELERATION_SWITCH_THRESHOLD) < 0)
        {
            madgwick_update_imu(&m_CurrentFrame.orientation, imuData.dt, m_MadgwickGain, imuData.gyro, imuData.acc);
        }
        else
        {
            integrate_gyro(&m_CurrentFrame.orientation, imuData.gyro, imuData.dt);
        }

        if (m_HighGainDisableTimer == 0)
        {
            vec3_t newAcc = imuData.acc;
            quat_rotate_vec(&newAcc, &m_CurrentFrame.orientation);
            newAcc.x *= -1;
            newAcc.y *= -1;
            newAcc.z -= EARTH_GRAVITY;

            m_CurrentFrame.acceleration = newAcc;

            EKFControls controls;
            controls.acc = newAcc.z;

            EKFMeasurements measurements;
            measurements.gps_alt = m_NEDPos.z;
            measurements.baroHeight = m_BaroHeight;

            m_EKF.predictState(controls, imuData.dt);
            m_EKF.predictCovariance(controls, imuData.dt);
            m_EKF.fusion(measurements);
            m_EKF.forceSymmetry();

            m_CurrentFrame.position.x = m_NEDPos.x;
            m_CurrentFrame.position.y = m_NEDPos.y;
            m_CurrentFrame.position.z = m_EKF.getState().pos;
            m_CurrentFrame.velocity.x = 0;
            m_CurrentFrame.velocity.y = 0;
            m_CurrentFrame.velocity.z = m_EKF.getState().vel;

            m_EKFVars.varGPS = EKF_OUTDATED_MEASUREMENT_VARIANCE;
            m_EKFVars.varBar = EKF_OUTDATED_MEASUREMENT_VARIANCE;
            m_EKF.setVariances(m_EKFVars);
        }

        m_EKFPublisher.publish(m_CurrentFrame);
    }
}