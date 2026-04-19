H[0][0] = 1;
H[0][1] = 0;
H[1][0] = 1;
H[1][1] = 0;


const float KS0 = 1.0f/(data->P[0][0] + var_gps);
const float KS1 = 1.0f/(data->P[0][0] + var_baro);

K[0][0] = KS0*data->P[0][0];
K[0][1] = KS1*data->P[0][0];
K[1][0] = KS0*data->P[1][0];
K[1][1] = KS1*data->P[1][0];


