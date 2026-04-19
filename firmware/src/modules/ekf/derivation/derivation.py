from sympy import *
import code_gen


def run_derivation():
    # ======== Init ========

    dt = symbols("dt")

    pos = symbols("pos")
    vel = symbols("vel")
    acc = symbols("acc")

    # ======== State Vector ========

    state_vector = Matrix(
        [
            pos,
            vel,
        ]
    )

    # ======== Control Vector ========

    control_vector = Matrix(
        [
            acc,
        ]
    )

    # ======== State Transition Function ========

    f = Matrix(
        [
            pos + vel * dt + acc * dt**2 / 2,
            vel + acc * dt,
        ]
    )
    F = f.jacobian(state_vector)
    G = f.jacobian(control_vector)
    P = Matrix(state_vector.shape[0], state_vector.shape[0], lambda i, j: Symbol(
        "P[" + str(i) + "][" + str(j) + "]", real=True))

    # ======== Process Noise Covariance Matrix ========

    var_acc = symbols("var_acc")

    E = Matrix.diag(var_acc)
    Q = G * E * G.T

    # ======== State Covariance Matrix ========

    P_new = F * P * F.T + Q

    # ======== Observation Function ========

    h = Matrix(
        [
            pos,
            pos,
        ]
    )
    H = h.jacobian(state_vector)

    # ======== Observation Noise Covariance Matrix ========

    var_gps, var_baro = symbols("var_gps, var_baro")

    R = Matrix.diag(var_gps, var_baro)

    # ======== Kalman Gain ========

    K = zeros(H.shape[1], H.shape[0])

    H_temp = H[0, :]
    innov_var = H_temp * P * H_temp.T + Matrix([R[0, 0]])
    K[:, 0] = P * H_temp.T / innov_var[0, 0]

    H_temp = H[1, :]
    innov_var = H_temp * P * H_temp.T + Matrix([R[1, 1]])
    K[:, 1] = P * H_temp.T / innov_var[0, 0]

    # ======== Final ========

    code_gen.write_cov_matrix("covariance_prediction", P_new)
    code_gen.write_obs_eqs("fusion", (H, K))


if __name__ == "__main__":
    run_derivation()
