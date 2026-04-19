#pragma once

#include "ekf.h"
#include "madgwick.h"
#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>

class EKFModule
{
public:
    void init();
    void run();

private:
    PubSub::Subscriber<PubSub::Topics::SensorsIMU> m_IMUSubscriber{PUBSUB_ID(sensors_imu_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsBaro> m_BaroSubscriber{PUBSUB_ID(sensors_baro_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsGPS> m_GPSSubscriber{PUBSUB_ID(sensors_gps_1)};
    PubSub::Publisher<PubSub::Topics::EKFState> m_EKFPublisher{PUBSUB_ID(ekf_state)};

    EKF m_EKF;
    EKFVariances m_EKFVars;
    PubSub::Topics::EKFState m_CurrentFrame;
    float m_MadgwickGain;
    geo_position_wgs84_t m_BaseGPSPos;
    vec3_prec_t m_NEDPos;
    float m_BaroHeightOffset;
    float m_BaroHeight;
    uint32_t m_HighGainDisableTimer;

    void updateGPSData();
    void updateBarometerData();
    void updateIMUData();
};