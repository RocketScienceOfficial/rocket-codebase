import numpy as np
import matplotlib.pyplot as plt
from sim import datalink
from sim.env.physics_engines import PhysicsEngineOutput


def plot(received_data: list[datalink.sitl_response_data], true_data: list[PhysicsEngineOutput], dt: float):
    rec_pos = np.array([[d.posN, d.posE, d.posD] for d in received_data], dtype=float)
    rec_vel = np.array([[d.velN, d.velE, d.velD] for d in received_data], dtype=float)
    rec_q = np.array([[d.qw, d.qx, d.qy, d.qz] for d in received_data], dtype=float)
    true_pos = np.array([state.pos for state in true_data], dtype=float) if None not in true_data else None
    true_vel = np.array([state.vel for state in true_data], dtype=float) if None not in true_data else None
    true_q = np.array([state.q for state in true_data], dtype=float) if None not in true_data else None

    time = np.arange(len(received_data), dtype=float) * dt
    
    fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)

    plot_specs = [
        (axes[0], true_vel, rec_vel, "Velocity", "m/s", ("N", "E", "D"), ("#d62728", "#1f77b4", "#2ca02c")),
        (axes[1], true_pos, rec_pos, "Position", "m", ("N", "E", "D"), ("#d62728", "#1f77b4", "#2ca02c")),
        (axes[2], true_q, rec_q, "Attitude (quaternions)", "unitless", ("W", "X", "Y", "Z"), ("#d62728", "#1f77b4", "#2ca02c", "#ff7f0e")),
    ]

    for ax, true_vals, received_vals, title, unit, labels, colors in plot_specs:
        for i, (label, color) in enumerate(zip(labels, colors)):
            if true_vals is not None:
                ax.plot(time, true_vals[:, i], label=f"True {label}", color=color, linestyle="--", linewidth=1.4)

            ax.plot(time, received_vals[:, i], label=f"Received {label}", color=color, linewidth=1.4)

        ax.set_title(title)
        ax.set_ylabel(unit)
        ax.grid(True, linestyle="--", alpha=0.5)
        ax.legend(loc="upper right")

    axes[-1].set_xlabel("Time (s)")
    fig.tight_layout()
