import numpy as np
import matplotlib.pyplot as plt
from sim import datalink
from sim.utils.plot_utils import get_ekf_filtered_arrays


def plot(received_data: list[datalink.sitl_response_data], dt: float):
    time_data, filtered_received_data, filtered_true_data = get_ekf_filtered_arrays(received_data, None, dt)

    fig, (ax_top, ax_bottom) = plt.subplots(2, 1, figsize=(14, 10), sharex=True)

    height = np.array([-data.posD for data in filtered_received_data], dtype=float)
    predicted_apogee = np.array([data.predictedApogee for data in filtered_received_data], dtype=float)

    transition_time = None

    for i in range(1, len(filtered_received_data)):
        if (filtered_received_data[i - 1].smState == datalink.state_machine_state.DATALINK_SM_STATE_ACCELERATING and filtered_received_data[i].smState == datalink.state_machine_state.DATALINK_SM_STATE_FREE_FLIGHT):
            transition_time = time_data[i]
            break

    actual_apogee = float(np.max(height))
    apogee_error = predicted_apogee - actual_apogee

    print("Apogee prediction absolute error: {:.2f} m".format(np.max(predicted_apogee) - actual_apogee))

    ax_top.plot(time_data, height, color="#1f77b4", linewidth=1.6, label="Actual Height")
    ax_top.plot(time_data, predicted_apogee, color="#ff7f0e", linewidth=1.6, label="Predicted Apogee")

    if transition_time is not None:
        ax_top.axvline(
            x=transition_time,
            color="#2ca02c",
            linestyle="--",
            linewidth=1.3,
            alpha=0.9,
            label="Accelerating -> Free Flight",
        )

    ax_top.set_title("Actual Height and Predicted Apogee vs Time", fontsize=14)
    ax_top.set_ylabel("Height (m)", fontsize=12)
    ax_top.grid(True, linestyle="--", alpha=0.6)
    ax_top.legend(loc="upper right")

    ax_bottom.plot(time_data, apogee_error, color="#d62728", linewidth=1.6, label="Predicted - Actual Apogee")
    ax_bottom.axhline(y=0.0, color="#1f1f1f", linestyle="--", linewidth=1.2, alpha=0.8, label="Zero Error")

    if transition_time is not None:
        ax_bottom.axvline(
            x=transition_time,
            color="#2ca02c",
            linestyle="--",
            linewidth=1.3,
            alpha=0.9,
            label="Accelerating -> Free Flight",
        )

    ax_bottom.set_title("Predicted Apogee Error vs Time", fontsize=14)
    ax_bottom.set_xlabel("Time (s)", fontsize=12)
    ax_bottom.set_ylabel("Apogee Error (m)", fontsize=12)
    ax_bottom.grid(True, linestyle="--", alpha=0.6)
    ax_bottom.legend(loc="upper right")

    fig.tight_layout()
