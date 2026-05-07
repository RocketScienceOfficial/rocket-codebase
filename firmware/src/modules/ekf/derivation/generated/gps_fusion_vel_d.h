#pragma once

namespace gen {

void gps_fusion_vel_d(
    const float *state,
    const CovarianceMatrix<EKF_NUM_ERROR_STATES>& P,
    float meas,
    float var_gps_vel,
    float* innov,
    float* innov_var,
    float* H,
    float* K
) {
    const float x0 = P(8, 8) + var_gps_vel;
    const float x1 = 1.0/x0;
    *innov = meas - state[9];
    *innov_var = x0;
    H[0] = 0;
    H[1] = 0;
    H[2] = 0;
    H[3] = 0;
    H[4] = 0;
    H[5] = 0;
    H[6] = 0;
    H[7] = 0;
    H[8] = 1;
    H[9] = 0;
    H[10] = 0;
    H[11] = 0;
    H[12] = 0;
    H[13] = 0;
    H[14] = 0;
    H[15] = 0;
    H[16] = 0;
    H[17] = 0;
    H[18] = 0;
    H[19] = 0;
    H[20] = 0;
    K[0] = P(0, 8)*x1;
    K[1] = P(1, 8)*x1;
    K[2] = P(2, 8)*x1;
    K[3] = P(3, 8)*x1;
    K[4] = P(4, 8)*x1;
    K[5] = P(5, 8)*x1;
    K[6] = P(6, 8)*x1;
    K[7] = P(7, 8)*x1;
    K[8] = P(8, 8)*x1;
    K[9] = P(9, 8)*x1;
    K[10] = P(10, 8)*x1;
    K[11] = P(11, 8)*x1;
    K[12] = P(12, 8)*x1;
    K[13] = P(13, 8)*x1;
    K[14] = P(14, 8)*x1;
    K[15] = P(15, 8)*x1;
    K[16] = P(16, 8)*x1;
    K[17] = P(17, 8)*x1;
    K[18] = P(18, 8)*x1;
    K[19] = P(19, 8)*x1;
    K[20] = P(20, 8)*x1;
}
}