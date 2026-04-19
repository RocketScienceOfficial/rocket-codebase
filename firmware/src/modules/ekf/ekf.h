#pragma once

#define EKF_NUM_STATES 2
#define EKF_NUM_CONTROLS 1
#define EKF_NUM_MEASUREMENTS 2

struct EKFVariances
{
    float varAcc;
    float varGPS;
    float varBar;
};

struct EKFControls
{
    float acc;
};

struct EKFMeasurements
{
    float gps_alt;
    float baroHeight;
};

struct EKFState
{
    float pos;
    float vel;
};

class EKF
{
public:
    void init();
    void setVariances(const EKFVariances &vars) { variances = vars; }

    void predictState(const EKFControls &controls, float dt);
    void predictCovariance(const EKFControls &controls, float dt);
    void fusion(const EKFMeasurements &measurements);
    void forceSymmetry();

    const EKFState &getState() const { return x; }

private:
    EKFState x;
    EKFVariances variances;
    float P[EKF_NUM_STATES][EKF_NUM_STATES];
    float P_Next[EKF_NUM_STATES][EKF_NUM_STATES];
};