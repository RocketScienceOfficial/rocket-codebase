#pragma once

namespace gen {

void gps_fusion_vel_e(
    const float *state,
    const CovarianceMatrix<EKF_NUM_ERROR_STATES>& P,
    float meas,
    float var_gps_vel,
    float* innov,
    float* innov_var,
    float* H,
    float* K
) {
    const float x0 = P(7, 7) + var_gps_vel;
    const float x1 = 1.0/x0;
    *innov = meas - state[8];
    *innov_var = x0;
    H[0] = 0;
    H[1] = 0;
    H[2] = 0;
    H[3] = 0;
    H[4] = 0;
    H[5] = 0;
    H[6] = 0;
    H[7] = 1;
    H[8] = 0;
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
    K[0] = P(0, 7)*x1;
    K[1] = P(1, 7)*x1;
    K[2] = P(2, 7)*x1;
    K[3] = P(3, 7)*x1;
    K[4] = P(4, 7)*x1;
    K[5] = P(5, 7)*x1;
    K[6] = P(6, 7)*x1;
    K[7] = P(7, 7)*x1;
    K[8] = P(8, 7)*x1;
    K[9] = P(9, 7)*x1;
    K[10] = P(10, 7)*x1;
    K[11] = P(11, 7)*x1;
    K[12] = P(12, 7)*x1;
    K[13] = P(13, 7)*x1;
    K[14] = P(14, 7)*x1;
    K[15] = P(15, 7)*x1;
    K[16] = P(16, 7)*x1;
    K[17] = P(17, 7)*x1;
    K[18] = P(18, 7)*x1;
    K[19] = P(19, 7)*x1;
    K[20] = P(20, 7)*x1;
}
}