#pragma once

#include "EKFData.h"
#include "utils/CovarianceMatrix.h"

class EKF
{
public:
    void init();

    void predictState(const EKFIMUData &imu);
    void predictCovariance(const EKFIMUData &imu);

    bool fuseGPSPosition(const EKFGPSPosMeasurement &meas, float gate_threshold);
    bool fuseGPSVelocity(const EKFGPSVelMeasurement &meas, float gate_threshold);
    bool fuseBaroHeight(const EKFBaroMeasurement &meas, float gate_threshold);

    EKFNominalState &getState() { return m_NominalState; }
    float &getCovarianceElement(size_t i, size_t j) { return P_current(i, j); }

private:
    EKFNominalState m_NominalState;
    EKFErrorState m_ErrorState;

    CovarianceMatrix<EKF_NUM_ERROR_STATES> P_current;
    CovarianceMatrix<EKF_NUM_ERROR_STATES> P_next;

    float _K[EKF_NUM_ERROR_STATES];
    float _H[EKF_NUM_ERROR_STATES];
    float _HP[EKF_NUM_ERROR_STATES];

    void applyFusion(float innov);
    void updateCovariancePostFusion();

    bool shouldFuseMeasurement(float innov, float innov_var, float gate_threshold);

    void injectErrorState();
    void resetErrorState();
    void updateErrorState(float innov);
};