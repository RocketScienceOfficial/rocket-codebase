import numpy as np
import matplotlib.pyplot as plt
from sim import datalink
from sim.env.physics_engines import PhysicsEngineOutput


def plot(received_data: list[datalink.sitl_response_data], true_data: list[PhysicsEngineOutput], dt: float):
    rec_pos = np.array([[d.posN, d.posE, d.posD] for d in received_data], dtype=float)
    rec_vel = np.array([[d.velN, d.velE, d.velD] for d in received_data], dtype=float)
    rec_acc = np.array([[d.accN, d.accE, d.accD] for d in received_data], dtype=float)
    true_pos = np.array([state.pos for state in true_data], dtype=float) if None not in true_data else None
    true_vel = np.array([state.vel for state in true_data], dtype=float) if None not in true_data else None
    true_acc = np.array([state.acc for state in true_data], dtype=float) if None not in true_data else None

    time = np.arange(len(received_data), dtype=float) * dt

    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)
    axis_labels = ("N", "E", "D")
    colors = ("#d62728", "#1f77b4", "#2ca02c")

    plot_specs = [
        (axes[0], true_acc, rec_acc, "Acceleration", "m/s^2"),
        (axes[1], true_vel, rec_vel, "Velocity", "m/s"),
        (axes[2], true_pos, rec_pos, "Position", "m"),
    ]

    for ax, true_vals, received_vals, title, unit in plot_specs:
        for i, (label, color) in enumerate(zip(axis_labels, colors)):
            if true_vals is not None:
                ax.plot(time, true_vals[:, i], label=f"True {label}", color=color, linestyle="--", linewidth=1.4)
            
            ax.plot(time, received_vals[:, i], label=f"Received {label}", color=color, linewidth=1.4)

        ax.set_title(title)
        ax.set_ylabel(unit)
        ax.grid(True, linestyle="--", alpha=0.5)
        ax.legend(loc="upper right")

    axes[-1].set_xlabel("Time (s)")
    fig.tight_layout()

    plt.pause(0.1)
    plt.show()
