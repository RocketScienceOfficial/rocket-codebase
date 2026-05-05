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
    sim_dt_per_frame = screen_dt * speedup_factor
    step = max(1, int(sim_dt_per_frame / dt))

    q_est_down = [np.array([d.qx, d.qy, d.qz, d.qw]) for d in received_data if [d.qx, d.qy, d.qz, d.qw] != [0, 0, 0, 0]][::step]
    q_true_down = [np.array([state.q[1], state.q[2], state.q[3], state.q[0]]) for state in true_data if state is not None][::step] if None not in true_data else None

    interval_ms = int(screen_dt * 1000)
    num_frames = len(q_est_down)

    print(f"Data recorded at {1/dt:.0f}Hz.")
    print(f"Playing back at {speedup_factor}x speed (Skipping {step} frames per visual update).")

    fig = plt.figure(figsize=(8, 8))
    ax = fig.add_subplot(111, projection='3d')

    origin = np.array([0, 0, 0])
    base_axes = np.eye(3)
    colors = ['r', 'g', 'b']

    quivers_true = [None, None, None]
    quivers_est = [None, None, None]

    ax.plot([], [], [], color='black', linestyle='dashed', alpha=0.6, label='True')
    ax.plot([], [], [], color='black', linestyle='solid', label='Estimate')
    ax.legend(loc='upper right')

    ax.set(xlim=[-1.5, 1.5], ylim=[-1.5, 1.5], zlim=[-1.5, 1.5], xlabel='X (Red)', ylabel='Y (Green)', zlabel='Z (Blue)', title='Attitude')

    time_text = ax.text2D(0.05, 0.95, '', transform=ax.transAxes, fontsize=12, fontweight='bold')

    def update(frame):
        rot_est = R.from_quat(q_est_down[frame])
        axes_est = rot_est.apply(base_axes)

        if q_true_down is not None:
            rot_true = R.from_quat(q_true_down[frame])
            axes_true = rot_true.apply(base_axes)

        updated_artists = []

        current_time_sec = frame * step * dt
        time_text.set_text(f"Time: {current_time_sec:.3f} s")
        updated_artists.append(time_text)

        for i in range(3):
            # Draw True (Solid, Thick)
            if q_true_down is not None:
                if quivers_true[i] is not None:
                    quivers_true[i].remove()
                quivers_true[i] = ax.quiver(origin[0], origin[1], origin[2], axes_true[0, i], axes_true[1, i], axes_true[2, i], color=colors[i], linestyle='dashed', linewidth=2, alpha=0.5, normalize=True)
                updated_artists.append(quivers_true[i])

            # Draw Estimated (Dashed, Transparent)
            if quivers_est[i] is not None:
                quivers_est[i].remove()
            quivers_est[i] = ax.quiver(origin[0], origin[1], origin[2], axes_est[0, i], axes_est[1, i], axes_est[2, i], color=colors[i], linestyle='solid', linewidth=3, normalize=True)
            updated_artists.append(quivers_est[i])

        return updated_artists

    ani = animation.FuncAnimation(fig, update, frames=num_frames, interval=interval_ms, blit=False, repeat=False)

    plt.show()

    return ani
