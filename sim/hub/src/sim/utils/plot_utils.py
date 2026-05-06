import numpy as np
from sim import datalink
from sim.env.physics_engines import PhysicsEngineOutput


def get_ekf_filtered_arrays(received_data: list[datalink.sitl_response_data], true_data: list[PhysicsEngineOutput], dt: float):
    ekf_flag = datalink.sitl_response_flags.DATALINK_FLAGS_SITL_RESP_EKF
    valid_indices = [i for i, rec in enumerate(received_data) if rec is not None and (rec.responseFlags & ekf_flag) > 0]

    if len(valid_indices) == 0:
        print("No EKF-flagged data to filter.")
        return None, None, None

    time_data = np.array(valid_indices, dtype=float) * dt
    filtered_received_data = [received_data[i] for i in valid_indices]
    filtered_true_data = [true_data[i] for i in valid_indices] if true_data is not None and None not in true_data else None

    return time_data, filtered_received_data, filtered_true_data
