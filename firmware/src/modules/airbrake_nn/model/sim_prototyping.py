import math
import os
import pandas as pd
import numpy as np
from numpy.polynomial import Polynomial
import matplotlib.pyplot as plt


cd_df = pd.read_csv(os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "taipan_or_data.csv"))
cd_df = cd_df[cd_df["Vertical velocity (m/s)"] > 0.1]
cd_df = cd_df[cd_df["# Time (s)"] > 6.44]

cd_mach = cd_df["Mach number ()"].values
cd_data = cd_df["Drag coefficient ()"].values
cd_p = Polynomial.fit(cd_mach, cd_data, 3)


def get_cd(h, v):
    a = np.sqrt(1.4 * 287.05 * (288.15 - 0.0065 * h))
    mach = v / a
    return cd_p(mach)


def rocket_derivatives(t, state, thrust_accel, burnout_time, mass, area, cd):
    h, v = state
    g = 9.8085

    rho_0 = 1.225
    scale_height = 8500
    rho = rho_0 * np.pow(1 - 0.0065 * h / 288.15, 4.25588)

    cd = get_cd(h, v)
    f_drag = 0.5 * rho * v * abs(v) * cd * area
    a_drag = f_drag / mass

    # 3. Determine Net Acceleration
    if t <= burnout_time:
        a_net = thrust_accel - g - a_drag  # Powered flight
    else:
        a_net = -g - a_drag                # Coasting phase

    return [v, a_net]


def simulate_to_apogee(start_h, start_v, thrust_accel, burnout_time, mass, area, cd, dt=0.01):
    t = 0.0
    state = [start_h, start_v]
    trajectory_cache = [(state[0], state[1])]

    while True:
        # Pass the extra aerodynamic parameters into the RK4 evaluations
        args = (thrust_accel, burnout_time, mass, area, cd)

        # k1
        k1 = rocket_derivatives(t, state, *args)

        # k2
        state_k2 = [state[0] + 0.5 * dt * k1[0], state[1] + 0.5 * dt * k1[1]]
        k2 = rocket_derivatives(t + 0.5 * dt, state_k2, *args)

        # k3
        state_k3 = [state[0] + 0.5 * dt * k2[0], state[1] + 0.5 * dt * k2[1]]
        k3 = rocket_derivatives(t + 0.5 * dt, state_k3, *args)

        # k4
        state_k4 = [state[0] + dt * k3[0], state[1] + dt * k3[1]]
        k4 = rocket_derivatives(t + dt, state_k4, *args)

        # Calculate next state
        next_h = state[0] + (dt / 6.0) * (k1[0] + 2*k2[0] + 2*k3[0] + k4[0])
        next_v = state[1] + (dt / 6.0) * (k1[1] + 2*k2[1] + 2*k3[1] + k4[1])

        t += dt
        trajectory_cache.append((next_h, next_v))

        # Apogee check: coasting and velocity hits 0
        if t > burnout_time and next_v <= 0:
            break

        state = [next_h, next_v]

    return trajectory_cache


if __name__ == "__main__":
    start_h = 1054.9075
    start_v = 229.1248
    accel = 0.0
    burn_t = 0.0
    m = 9.820
    A = 0.01247
    Cd = 0.461

    results_drag = simulate_to_apogee(start_h, start_v, accel, burn_t, m, A, Cd)
    apogee_drag = results_drag[-1][0]

    print(max(results_drag, key=lambda x: x[1]))

    print(f"Apogee (With Drag): {apogee_drag:6.2f} m")
