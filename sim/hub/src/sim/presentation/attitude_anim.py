import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from scipy.spatial.transform import Rotation as R
from sim import datalink
from sim.env.physics_engines import PhysicsEngineOutput


matplotlib.use('TkAgg')


def animate_quaternions(received_data: list[datalink.sitl_response_data], true_data: list[PhysicsEngineOutput], dt: float, target_fps: int = 30, speedup_factor: float = 5.0):
    screen_dt = 1.0 / target_fps
    interval_ms = int(screen_dt * 1000)
    sim_dt_per_frame = screen_dt * speedup_factor
    step = max(1, int(sim_dt_per_frame / dt))

    # We use quaternion in x,y,z,w order
    valid_indexes = [i for i, d in enumerate(received_data) if [d.qx, d.qy, d.qz, d.qw] != [0, 0, 0, 0]]
    q_est_down = [np.array([d.qx, d.qy, d.qz, d.qw]) for d in [received_data[i] for i in valid_indexes]][::step]
    q_true_down = [np.array([d.q[1], d.q[2], d.q[3], d.q[0]]) for d in [true_data[i] for i in valid_indexes]][::step] if None not in true_data else None

    num_frames = len(q_est_down)

    print(f"Data recorded at {1/dt:.0f}Hz.")
    print(f"Playing back at {speedup_factor}x speed (Skipping {step} frames per visual update).")

    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    colors = ['r', 'g', 'b']

    def update(frame_index):
        ax.clear()
        ax.plot([], [], [], color='black', linestyle='dashed', alpha=0.6, label='True')
        ax.plot([], [], [], color='black', linestyle='solid', label='Estimate')
        ax.set(xlim=[-1.5, 1.5], ylim=[-1.5, 1.5], zlim=[-1.5, 1.5], xlabel='X (Red)', ylabel='Y (Green)', zlabel='Z (Blue)', title='Attitude')
        ax.set_title(f"Time: {frame_index * step * dt:.3f} s", loc='left', fontsize=12, fontweight='bold')
        ax.legend(loc='upper right')

        data = [
            (q_est_down[frame_index], 'solid'),
        ]

        if q_true_down is not None:
            data.append((q_true_down[frame_index], 'dashed'))

        for q, linestyle in data:
            # Get the rotation matrix
            rot = R.from_quat(q).as_matrix()

            # The columns of the rotation matrix represent the rotated X, Y, Z basis vectors
            v_x = rot[:, 0]
            v_y = rot[:, 1]
            v_z = rot[:, 2]

            # 1. Draw the main lines of the axes (linewidth controls thickness)
            for v, color in zip([v_x, v_y, v_z], colors):
                ax.plot([0, v[0]], [0, v[1]], [0, v[2]], color=color, linestyle=linestyle, linewidth=3)

            # 2. Draw a dot at the tip of each axis so you know which way it's pointing
            # for v, color in zip([v_x, v_y, v_z], colors):
            #     ax.plot([v[0]], [v[1]], [v[2]], color=color, marker='o', markersize=10)

    ani = animation.FuncAnimation(fig, update, frames=num_frames, interval=interval_ms, blit=False, repeat=True)

    plt.show()

    return ani
