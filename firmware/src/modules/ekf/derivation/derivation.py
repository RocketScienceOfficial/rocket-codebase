from sympy import *
from derivation_utils import *


def predict_covariance(true_state, state, error_state, P, dt):
    g = Matrix([0, 0, symbols("g")])

    accel = Matrix(symbols("accel_x accel_y accel_z"))
    gyro = Matrix(symbols("gyro_x gyro_y gyro_z"))

    # True state prediction derivation
    noise = {
        "accel": Matrix(symbols("noise_accel_x noise_accel_y noise_accel_z")),
        "gyro": Matrix(symbols("noise_gyro_x noise_gyro_y noise_gyro_z")),
    }

    input_true = {
        "accel": accel - true_state["bias_accel"] - noise["accel"],
        "gyro": gyro - true_state["bias_gyro"] - noise["gyro"]
    }
    rot_true_state = quaternion_rot_matrix(true_state["q"])
    true_state_pred = true_state.copy()
    true_state_pred["q"] = quaternion_multiply(true_state["q"], [1, input_true["gyro"][0] * dt / 2, input_true["gyro"][1] * dt / 2, input_true["gyro"][2] * dt / 2])  # using small angle approximation
    true_state_pred["pos"] = true_state["pos"] + true_state["vel"] * dt
    true_state_pred["vel"] = true_state["vel"] + (rot_true_state * input_true["accel"] + g) * dt

    # Nominal state prediction derivation
    input_nominal = {
        "accel": accel - state["bias_accel"],
        "gyro": gyro - state["bias_gyro"]
    }
    rot_nominal_state = quaternion_rot_matrix(state["q"])
    nominal_state_pred = state.copy()
    nominal_state_pred["q"] = quaternion_multiply(state["q"], [1, input_nominal["gyro"][0] * dt / 2, input_nominal["gyro"][1] * dt / 2, input_nominal["gyro"][2] * dt / 2])  # using small angle approximation
    nominal_state_pred["pos"] = state["pos"] + state["vel"] * dt
    nominal_state_pred["vel"] = state["vel"] + (rot_nominal_state * input_nominal["accel"] + g) * dt

    # Error state prediction derivation
    error_state_pred = {}
    for key in error_state.keys():
        if key == "theta":
            d_q = quaternion_multiply(true_state_pred["q"], quaternion_conjugate(nominal_state_pred["q"]))  # q_nominal -> q_true
            error_state_pred[key] = 2 * Matrix([d_q[1], d_q[2], d_q[3]])  # using small angle approximation; the factor of 2 is because of the division by 2 in the quaternion multiplication for the state prediction
        else:
            error_state_pred[key] = true_state_pred[key] - nominal_state_pred[key]

    # Optimize error state prediction by removing higher order terms and factoring out the error state variables
    for i in range(len(error_state_pred["theta"])):
        error_state_pred["theta"][i] = expand(error_state_pred["theta"][i]).subs(dt**2, 0)
        q_e_w, q_e_x, q_e_y, q_e_z = state["q"]
        error_state_pred["theta"][i] = factor(error_state_pred["theta"][i], error_state["theta"][i]).subs(2*q_e_w**2 + 2*q_e_x**2 + 2*q_e_y**2 + 2*q_e_z**2, 2*1)

    # Zero the error state and noise variables to get the Jacobians
    zero_state_error = {error_state[key][i]: 0 for key in error_state.keys() for i in range(error_state[key].shape[0])}
    zero_noise = {noise[key][i]: 0 for key in noise.keys() for i in range(noise[key].shape[0])}
    error_state_pred_vector = Matrix(list(error_state_pred.values()))

    # Compute the Jacobians and the covariance prediction
    A = error_state_pred_vector.jacobian(Matrix(list(error_state.values()))).subs(zero_state_error).subs(zero_noise)
    G = error_state_pred_vector.jacobian(Matrix(list(noise.values()))).subs(zero_state_error).subs(zero_noise)

    var_acc, var_gyro = symbols("var_acc var_gyro")
    var_u = Matrix.diag([var_acc, var_acc, var_acc, var_gyro, var_gyro, var_gyro])
    P_new = A * P * A.T + G * var_u * G.T

    # Since the covariance matrix is symmetric, we can set the lower triangular part to 0 to optimize the generated code
    for i in range(P_new.shape[0]):
        for j in range(P_new.shape[1]):
            if i > j:
                P_new[i, j] = 0

    write_cov_matrix("covariance_prediction", state, P_new)


def err_jacobian(h, true_state, state, error_state):
    true_state_vector = Matrix(list(true_state.values()))
    state_vector = Matrix(list(state.values()))
    error_state_vector = Matrix(list(error_state.values()))

    # Apply the chain rule to get the Jacobian of h wrt. the error state vector
    dh_dx = h.jacobian(state_vector)
    dx_derr = true_state_vector.jacobian(error_state_vector)
    H = dh_dx * dx_derr

    return H


def gen_obs_eq(name, P, true_state, state, error_state, h, var_obs):
    # Jacobian
    H = err_jacobian(h, true_state, state, error_state)

    # Innovation
    meas = Matrix([Symbol("meas", real=True)])
    innov = meas - h

    # Innovation variance
    innov_var = H * P * H.T + Matrix([[var_obs]])

    # Kalman gain
    gain = P * H.T / innov_var[0, 0]

    # Write the generated code
    write_obs_eqs(name, state, var_obs, innov, innov_var, H, gain)


def gps_fusion(true_state, state, err_state, P):
    h = Matrix([
        state["pos"],
        state["vel"],
    ])

    var_gps_pos = symbols("var_gps_pos")
    var_gps_vel = symbols("var_gps_vel")

    gen_obs_eq("gps_fusion_pos_n", P, true_state, state, err_state, h[0, :], var_gps_pos)
    gen_obs_eq("gps_fusion_pos_e", P, true_state, state, err_state, h[1, :], var_gps_pos)
    gen_obs_eq("gps_fusion_pos_d", P, true_state, state, err_state, h[2, :], var_gps_pos)
    gen_obs_eq("gps_fusion_vel_n", P, true_state, state, err_state, h[3, :], var_gps_vel)
    gen_obs_eq("gps_fusion_vel_e", P, true_state, state, err_state, h[4, :], var_gps_vel)
    gen_obs_eq("gps_fusion_vel_d", P, true_state, state, err_state, h[5, :], var_gps_vel)


def baro_fusion(true_state, state, err_state, P):
    h = Matrix([
        state["pos"][2],
    ])

    var_baro = symbols("var_baro")

    gen_obs_eq("baro_fusion", P, true_state, state, err_state, h[0, :], var_baro)


def mag_fusion(true_state, state, err_state, P):
    rot = quaternion_rot_matrix(state["q"]).T  # we rotate the magnetic field from the global frame to the body frame because the magnetometer measures the magnetic field in the body frame
    h = Matrix([
        rot * (state["mag"] + state["bias_mag"]),
    ])

    var_mag = symbols("var_mag")

    gen_obs_eq("mag_fusion_mag_n", P, true_state, state, err_state, h[0, :], var_mag)
    gen_obs_eq("mag_fusion_mag_e", P, true_state, state, err_state, h[1, :], var_mag)
    gen_obs_eq("mag_fusion_mag_d", P, true_state, state, err_state, h[2, :], var_mag)


# States definitions
state = {
    "q": Matrix(symbols("q_w q_x q_y q_z")),  # q represents the rotation from the body frame (FRD) to the global frame (NED)
    "pos": Matrix(symbols("pos_n pos_e pos_d")),
    "vel": Matrix(symbols("vel_n vel_e vel_d")),
    "mag": Matrix(symbols("mag_n mag_e mag_d")),
    "bias_gyro": Matrix(symbols("bias_gyro_x bias_gyro_y bias_gyro_z")),
    "bias_accel": Matrix(symbols("bias_accel_x bias_accel_y bias_accel_z")),
    "bias_mag": Matrix(symbols("bias_mag_x bias_mag_y bias_mag_z")),
}
error_state = {
    "theta": Matrix(symbols("d_theta_x d_theta_y d_theta_z")),
    "pos": Matrix(symbols("d_pos_n d_pos_e d_pos_d")),
    "vel": Matrix(symbols("d_vel_n d_vel_e d_vel_d")),
    "mag": Matrix(symbols("d_mag_n d_mag_e d_mag_d")),
    "bias_gyro": Matrix(symbols("d_bias_gyro_x d_bias_gyro_y d_bias_gyro_z")),
    "bias_accel": Matrix(symbols("d_bias_accel_x d_bias_accel_y d_bias_accel_z")),
    "bias_mag": Matrix(symbols("d_bias_mag_x d_bias_mag_y d_bias_mag_z")),
}
true_state = {
    "q": quaternion_multiply([1, error_state["theta"][0] / 2, error_state["theta"][1] / 2, error_state["theta"][2] / 2], state["q"]),  # we define error state "theta" in global frame (left hand multiplication); using small angle approximation
    "pos": state["pos"] + error_state["pos"],
    "vel": state["vel"] + error_state["vel"],
    "mag": state["mag"] + error_state["mag"],
    "bias_gyro": state["bias_gyro"] + error_state["bias_gyro"],
    "bias_accel": state["bias_accel"] + error_state["bias_accel"],
    "bias_mag": state["bias_mag"] + error_state["bias_mag"],
}

# dt
dt = symbols("dt")

# Covariance matrix definition
dim = sum([error_state[key].shape[0] for key in error_state.keys()])
P = Matrix(dim, dim, lambda i, j: Symbol("P(" + str(i) + ", " + str(j) + ")", real=True))

# Run derivation
predict_covariance(true_state, state, error_state, P, dt)
gps_fusion(true_state, state, error_state, P)
baro_fusion(true_state, state, error_state, P)
mag_fusion(true_state, state, error_state, P)

print("Derivation completed")
