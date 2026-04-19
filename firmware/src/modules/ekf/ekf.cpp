#include "ekf.h"
#include <cstddef>

void EKF::init()
{
    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j < EKF_NUM_STATES; j++)
        {
            P[i][j] = i == j ? 10.0f : 0.0f;
            P_Next[i][j] = 0.0f;
        }
    }
}

void EKF::predictState(const EKFControls &controls, float dt)
{
    EKFState newState;
    newState.pos = x.pos + x.vel * dt + 0.5f * controls.acc * dt * dt;
    newState.vel = x.vel + controls.acc * dt;

    x = newState;
}

void EKF::predictCovariance(const EKFControls &controls, float dt)
{
    (void)controls;

    const float var_acc = variances.varAcc;

    const float PS0 = P[1][1] * dt;
    const float PS1 = PS0 + P[0][1];
    const float PS2 = (1.0f / 2.0f) * var_acc * (dt * dt * dt);

    P_Next[0][0] = PS1 * dt + P[0][0] + P[1][0] * dt + (1.0f / 4.0f) * var_acc * (dt * dt * dt * dt);
    P_Next[0][1] = PS1 + PS2;
    P_Next[1][1] = P[1][1] + var_acc * (dt * dt);

    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j < EKF_NUM_STATES; j++)
        {
            if (j >= i)
            {
                P[i][j] = P_Next[i][j];
                P[j][i] = P_Next[i][j];
            }
        }
    }
}

void EKF::fusion(const EKFMeasurements &measurements)
{
    const float var_gps = variances.varGPS;
    const float var_baro = variances.varBar;

    float H[EKF_NUM_MEASUREMENTS][EKF_NUM_STATES];

    H[0][0] = 1;
    H[0][1] = 0;
    H[1][0] = 1;
    H[1][1] = 0;

    float K[EKF_NUM_STATES][EKF_NUM_MEASUREMENTS];

    const float KS0 = 1.0f / (P[0][0] + var_gps);
    const float KS1 = 1.0f / (P[0][0] + var_baro);

    K[0][0] = KS0 * P[0][0];
    K[0][1] = KS1 * P[0][0];
    K[1][0] = KS0 * P[1][0];
    K[1][1] = KS1 * P[1][0];

    const float *meas_data = (const float *)(&measurements);
    float *state_data = (float *)(&x);

    float innov[EKF_NUM_MEASUREMENTS];
    innov[0] = meas_data[0] - state_data[0];
    innov[1] = meas_data[1] - state_data[0];

    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j < EKF_NUM_MEASUREMENTS; j++)
        {
            state_data[i] += K[i][j] * innov[j];
        }
    }

    float I_KH[EKF_NUM_STATES][EKF_NUM_STATES];

    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j < EKF_NUM_STATES; j++)
        {
            float tmp = 0.0f;

            for (size_t k = 0; k < EKF_NUM_MEASUREMENTS; k++)
            {
                tmp += K[i][k] * H[k][j];
            }

            I_KH[i][j] = (i == j ? 1.0f : 0.0f) - tmp;
        }
    }

    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j < EKF_NUM_STATES; j++)
        {
            float tmp = 0.0f;

            for (size_t k = 0; k < EKF_NUM_STATES; k++)
            {
                tmp += I_KH[i][k] * P[k][j];
            }

            P_Next[i][j] = tmp;
        }
    }

    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j < EKF_NUM_STATES; j++)
        {
            P[i][j] = P_Next[i][j];
        }
    }
}

void EKF::forceSymmetry()
{
    for (size_t i = 0; i < EKF_NUM_STATES; i++)
    {
        for (size_t j = 0; j <= i; j++)
        {
            float tmp = (P[i][j] + P[j][i]) / 2.0f;

            P[i][j] = tmp;
            P[j][i] = tmp;
        }
    }
}