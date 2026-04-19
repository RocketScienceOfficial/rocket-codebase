const float PS0 = data->P[1][1]*dt;
const float PS1 = PS0 + data->P[0][1];
const float PS2 = (1.0f/2.0f)*var_acc*(dt*dt*dt);

data->P_Next[0][0] = PS1*dt + data->P[0][0] + data->P[1][0]*dt + (1.0f/4.0f)*var_acc*(dt*dt*dt*dt);
data->P_Next[0][1] = PS1 + PS2;
data->P_Next[1][1] = data->P[1][1] + var_acc*(dt*dt);


