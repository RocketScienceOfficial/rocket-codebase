#pragma once

#include <pubsub/Topics.h>
#include <pubsub/Publisher.h>
#include <pubsub/Subscriber.h>
#include "EKF.h"
#include "EKFData.h"
#include "EKFConfig.h"
#include "utils/RingBuffer.h"

class EKFModule
{
public:
    void init();
    void run();

private:
    // Interface
    PubSub::Subscriber<PubSub::Topics::SensorsIMU> m_IMUSubscriber{PUBSUB_ID(sensors_imu_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsBaro> m_BaroSubscriber{PUBSUB_ID(sensors_baro_1)};
    PubSub::Subscriber<PubSub::Topics::SensorsGPS> m_GPSSubscriber{PUBSUB_ID(sensors_gps_1)};
    PubSub::Publisher<PubSub::Topics::EKFState> m_EKFPublisher{PUBSUB_ID(ekf_state)};

    // EKF instance
    EKF m_EKF;
    bool m_EKFInitialized;
    bool m_EKFEnabled;
    uint32_t m_LastUpdateTime;

    // Output predictor state and buffer
    EKFNominalState m_CurrentOPState;
    EKFErrorState m_OPNextCorrection;
    RingBuffer<EKFNominalState, EKF_IMU_DELAY_HORIZON_SIZE> m_OutputPredictorBuffer;

    // IMU buffer
    vec3_t m_AccelAccum;
    vec3_t m_GyroAccum;
    float m_IMUDtAccum;
    size_t m_IMUSamplesCount;
    RingBuffer<EKFIMUData, EKF_IMU_DELAY_HORIZON_SIZE> m_IMUBuffer;

    // GPS origin
    geo_position_wgs84_t m_GPSOrigin;
    bool m_GPSOriginSet;
    RingBuffer<EKFGPSPosMeasurement, EKF_GPS_DELAY_HORIZON_SIZE> m_GPSPosBuffer;
    RingBuffer<EKFGPSVelMeasurement, EKF_GPS_DELAY_HORIZON_SIZE> m_GPSVelBuffer;

    // Barometer offset
    float m_BaroOffset;
    bool m_BaroOffsetSet;
    RingBuffer<EKFBaroMeasurement, EKF_BARO_DELAY_HORIZON_SIZE> m_BaroBuffer;

    // Processing functions
    void processIMU(const PubSub::Topics::SensorsIMU &imuData);
    void processGPS(const PubSub::Topics::SensorsGPS &gpsData);
    void processBaro(const PubSub::Topics::SensorsBaro &baroData);

    // Output predictor functions
    void outputPredictorIntegrate(const EKFIMUData &sample, uint32_t currentTime);
    void outputPredictorPublishState();
    void outputPredictorCalculateCorrection();

    // EKF update and fusion functions
    uint32_t getMinMeasurementTimestamp() const;
    void updateEKF();

    // Utility functions
    void initState();
    void initCovariance();
    void addBiasNoiseToCovariance(float dt);
    float gpsVarianceFromSatCount(uint8_t sats);
};