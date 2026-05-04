#pragma once

#include "EKFData.h"
#include "utils/CovarianceMatrix.h"

class EKF
{
public:
    void init();

    static void predictExplicitState(EKFNominalState &state, const EKFErrorState &error, const EKFIMUData &imu);

    void predictState(const EKFIMUData &imu);
    void predictCovariance(const EKFIMUData &imu);

    void fuseGPSPosition(const EKFGPSPosMeasurement &meas);
    void fuseGPSVelocity(const EKFGPSVelMeasurement &meas);
    void fuseBaroHeight(const EKFBaroMeasurement &meas);

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

    void applyFusion(float innov, float innov_var);
    void updateCovariancePostFusion();

    void injectErrorState();
    void resetErrorState();
    void updateErrorState(float innov);
};