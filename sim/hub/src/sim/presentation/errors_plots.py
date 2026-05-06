import numpy as np
import matplotlib.pyplot as plt
from sim import datalink
from sim.env.physics_engines import PhysicsEngineOutput
from sim.utils.plot_utils import get_ekf_filtered_arrays


def plot(received_data: list[datalink.sitl_response_data], true_data: list[PhysicsEngineOutput], dt: float):
    if None in true_data:
        print("No errors data to plot.")
        return

    time_data, filtered_received_data, filtered_true_data = get_ekf_filtered_arrays(received_data, true_data, dt)

    rec_pos = np.array([[d.posN, d.posE, d.posD] for d in filtered_received_data], dtype=float)
    rec_vel = np.array([[d.velN, d.velE, d.velD] for d in filtered_received_data], dtype=float)
    rec_q = np.array([[d.qw, d.qx, d.qy, d.qz] for d in filtered_received_data], dtype=float)
    true_pos = np.array([d.pos for d in filtered_true_data], dtype=float)
    true_vel = np.array([d.vel for d in filtered_true_data], dtype=float)
    true_q = np.array([d.q for d in filtered_true_data], dtype=float)

    pos_err = rec_pos - true_pos
    vel_err = rec_vel - true_vel
    q_err = rec_q - true_q

    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)

    plot_specs = [
        (axes[0], vel_err, "Velocity Error", "m/s", ("N", "E", "D"), ("#d62728", "#1f77b4", "#2ca02c")),
        (axes[1], pos_err, "Position Error", "m", ("N", "E", "D"), ("#d62728", "#1f77b4", "#2ca02c")),
        (axes[2], q_err, "Attitude Error (quaternions)", "unitless", ("W", "X", "Y", "Z"), ("#d62728", "#1f77b4", "#2ca02c", "#ff7f0e")),
    ]

    for ax, err, title, unit, label_list, color_list in plot_specs:
        for i, (label, color) in enumerate(zip(label_list, color_list)):
            ax.plot(time_data, err[:, i], label=label, color=color, linewidth=1.4)

        ax.set_title(title)
        ax.set_ylabel(unit)
        ax.grid(True, linestyle="--", alpha=0.5)
        ax.legend(loc="upper right")

    axes[-1].set_xlabel("Time (s)")
    fig.tight_layout()
