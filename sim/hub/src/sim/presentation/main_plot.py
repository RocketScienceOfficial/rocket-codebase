import numpy as np
import matplotlib.pyplot as plt
from sim import datalink
from sim.utils.plot_utils import get_ekf_filtered_arrays


def plot(received_data: list[datalink.sitl_response_data], dt: float):
    time_data, filtered_received_data, filtered_true_data = get_ekf_filtered_arrays(received_data, None, dt)

    height = np.array([-data.posD for data in filtered_received_data], dtype=float)

    standing_data = (datalink.state_machine_state.DATALINK_SM_STATE_STANDING, "Standing", "#3967e6")
    armed_data = (datalink.state_machine_state.DATALINK_SM_STATE_ARMED, "Armed", "#398ae6")
    accelerating_data = (datalink.state_machine_state.DATALINK_SM_STATE_ACCELERATING, "Accelerating", "#f11c1c")
    free_flight_data = (datalink.state_machine_state.DATALINK_SM_STATE_FREE_FLIGHT, "Free Flight", "#f3dd19")
    free_fall_data = (datalink.state_machine_state.DATALINK_SM_STATE_FREE_FALL, "Free Fall", "#31e421")
    landed_data = (datalink.state_machine_state.DATALINK_SM_STATE_LANDED, "Landed", "#822a9d")

    plt.figure(figsize=(20, 12))

    for state, label, color in [standing_data, armed_data, accelerating_data, free_flight_data, free_fall_data, landed_data]:
        mask = np.array([data.smState == state for data in filtered_received_data])
        plt.plot(time_data[mask], height[mask], color=color, label=label)

    ign1_data = (1 << 0, "Pilot Ignition", "#000000")
    ign2_data = (1 << 1, "Main Ignition", "#720068")
    ign3_data = (1 << 2, "Ignition 3", "#720068")
    ign4_data = (1 << 3, "Ignition 4", "#720068")

    for ign_flag, label, color in [ign1_data, ign2_data, ign3_data, ign4_data]:
        ign_time = [i for i, data in enumerate(filtered_received_data) if (data.ignFiredFlags & ign_flag) > 0]

        if len(ign_time) > 0:
            ign_fire_time = time_data[min(ign_time)]
            plt.axvline(x=ign_fire_time, color=color, linestyle='--', linewidth=1.5, alpha=0.7, label=label)

    plt.title("Height vs Time", fontsize=14)
    plt.xlabel("Time (s)", fontsize=12)
    plt.ylabel("Height (m)", fontsize=12)

    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(loc='upper left')
