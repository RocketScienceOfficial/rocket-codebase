import numpy as np
import matplotlib.pyplot as plt
from sim import datalink
from sim.env.physics_engines import PhysicsEngineOutput


def plot(received_data: list[datalink.sitl_response_data], true_data: list[PhysicsEngineOutput], dt: float):
    n = min(len(received_data), len([d for d in true_data if d is not None]))

    if n == 0 or n != len(received_data) or n != len(true_data):
        print("No errors data to plot.")
        return

    rec_pos = np.array([[d.posN, d.posE, d.posD] for d in received_data[:n]], dtype=float)
    rec_vel = np.array([[d.velN, d.velE, d.velD] for d in received_data[:n]], dtype=float)
    rec_q = np.array([[d.qw, d.qx, d.qy, d.qz] for d in received_data[:n]], dtype=float)
    true_pos = np.array([state.pos for state in true_data[:n]], dtype=float)
    true_vel = np.array([state.vel for state in true_data[:n]], dtype=float)
    true_q = np.array([state.q for state in true_data[:n]], dtype=float)

    pos_err = rec_pos - true_pos
    vel_err = rec_vel - true_vel
    q_err = rec_q - true_q
    time = np.arange(n, dtype=float) * dt

    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)

    plot_specs = [
        (axes[0], vel_err, "Velocity Error", "m/s", ("N", "E", "D"), ("#d62728", "#1f77b4", "#2ca02c")),
        (axes[1], pos_err, "Position Error", "m", ("N", "E", "D"), ("#d62728", "#1f77b4", "#2ca02c")),
        (axes[2], q_err, "Attitude Error (quaternions)", "unitless", ("W", "X", "Y", "Z"), ("#d62728", "#1f77b4", "#2ca02c", "#ff7f0e")),
    ]

    for ax, err, title, unit, label_list, color_list in plot_specs:
        for i, (label, color) in enumerate(zip(label_list, color_list)):
            ax.plot(time, err[:, i], label=label, color=color, linewidth=1.4)

        ax.set_title(title)
        ax.set_ylabel(unit)
        ax.grid(True, linestyle="--", alpha=0.5)
        ax.legend(loc="upper right")

    axes[-1].set_xlabel("Time (s)")
    fig.tight_layout()
