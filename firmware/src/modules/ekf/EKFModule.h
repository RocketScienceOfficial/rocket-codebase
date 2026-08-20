#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include <lib/geo/projection.h>
#include "ekf.h"
#include "EKFData.h"
#include "EKFConfig.h"
#include "utils/TimestampedRingBuffer.h"
#include "utils/RunningStats.h"

class EKFModule
{
public:
    void init();
    void run();

private:
    // Interface
    PubSub::Subscriber<PubSub::Topics::sensors_imu_1_topic> m_IMUSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_baro_1_topic> m_BaroSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_gps_1_topic> m_GPSSubscriber;
    PubSub::Subscriber<PubSub::Topics::sensors_mag_1_topic> m_MagSubscriber;
    PubSub::Publisher<PubSub::Topics::ekf_state_topic> m_EKFPublisher;

    // EKF instance
    EKF m_EKF;
    bool m_EKFInitialized;
    bool m_EKFEnabled;
    uint32_t m_LastUpdateTime;

    // Output Predictor
    EKFNominalState m_CurrentOPState;
    vec3_t m_AttitudeCorrection;
    TimestampedRingBuffer<EKFNominalState, EKF_DELAY_HORIZON_SIZE_IMU> m_OutputPredictorBuffer;

    // IMU
    vec3_t m_AccelAccum;
    vec3_t m_GyroAccum;
    uint8_t m_IMUClippingFlagsAccum;
    float m_IMUDtAccum;
    size_t m_IMUSamplesCount;
    TimestampedRingBuffer<EKFIMUData, EKF_DELAY_HORIZON_SIZE_IMU> m_IMUBuffer;

    // GPS
    equirect_projection_t m_Projection;
    bool m_GPSOriginSet;
    RunningStats<double> m_GPSNStats;
    RunningStats<double> m_GPSEStats;
    RunningStats<double> m_GPSDStats;
    bool m_GPSInitialized;
    TimestampedRingBuffer<EKFGPSPosMeasurement, EKF_DELAY_HORIZON_SIZE_GPS> m_GPSPosBuffer;
    TimestampedRingBuffer<EKFGPSVelMeasurement, EKF_DELAY_HORIZON_SIZE_GPS> m_GPSVelBuffer;

    // Barometer
    float m_BaroOffset;
    bool m_BaroOffsetSet;
    RunningStats<float> m_BaroStats;
    TimestampedRingBuffer<EKFBaroMeasurement, EKF_DELAY_HORIZON_SIZE_BARO> m_BaroBuffer;

    // Magnetometer
    RunningStats<float> m_MagXStats;
    RunningStats<float> m_MagYStats;
    RunningStats<float> m_MagZStats;
    bool m_MagInitDone;
    TimestampedRingBuffer<EKFMagMeasurement, EKF_DELAY_HORIZON_SIZE_MAG> m_MagBuffer;

    // Processing functions
    void processIMU(const PubSub::Messages::SensorsIMU &imuData);
    void processGPS(const PubSub::Messages::SensorsGPS &gpsData);
    void processBaro(const PubSub::Messages::SensorsBaro &baroData);
    void processMag(const PubSub::Messages::SensorsMag &magData);

    // Output predictor functions
    void outputPredictorForward(const EKFIMUData &sample, uint32_t currentTime);
    void outputPredictorCalculateState(const EKFIMUData &sample);
    void outputPredictorCalculateCorrection(float dt);

    // EKF update and fusion functions
    uint32_t getMinMeasurementTimestamp() const;
    void updateEKF();

    // Utility functions
    bool initState();
    void initCovariance();
    void yawReset();
    void addBiasNoiseToCovariance(float dt);
};