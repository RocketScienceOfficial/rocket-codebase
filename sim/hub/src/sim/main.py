import argparse
import time
import matplotlib.pyplot as plt
from . import datalink
from sim.connection.network import TCPSocket
from sim.presentation import axes_plots, errors_plots, main_plot, predicted_apogee_plot, attitude_anim
from sim.env.environments import *
from sim.env.physics_engines import *


# ============ SIMULATION CONFIGURATION ============
np.random.seed(42)

AUTO_ARM = True
SIM_TICK_DT = 0.001
# ==================================================


# ============ SOCKETS SETUP ============
physx_sock = TCPSocket(name="physx", ip="127.0.0.1", port=12345, is_server=False, blocking=True)
radio_sock = TCPSocket(name="radio", ip="127.0.0.1", port=12346, is_server=False, blocking=False)
# ================================================


# =========== ENVIRONMENT SETUP ============
parser = argparse.ArgumentParser(description="Python Rocket Simulation")
parser.add_argument("--config", type=str, required=True, help="Path to the configuration file")
args = parser.parse_args()

if args.config == "fm2024":
    from sim.configs.cfg_fm2024 import get_environment
elif args.config == "or_taipan":
    from sim.configs.cfg_or_taipan import get_environment
elif args.config == "simulink_acs":
    from sim.configs.cfg_simulink_acs import get_environment
elif args.config == "simulink_airbrake":
    from sim.configs.cfg_simulink_airbrake import get_environment
elif args.config == "rotating":
    from sim.configs.cfg_rotating import get_environment
else:
    raise ValueError(f"Unknown configuration: {args.config}")

env = get_environment(SIM_TICK_DT)
# ============================================


# ============ SIMULATION LOOP ============
if AUTO_ARM:
    packet = datalink.telemetry_response(
        cmd=datalink.telemetry_cmd.DATALINK_TELEMETRY_CMD_ARM,
        cmd_seq=1,
    )

    radio_sock.send(packet.pack())

    print("> Armed rocket.")

print("> Starting simulation...")

tick = 0
start_time = time.time()
received_data = []
true_data = []

env.forward(PhysicsEngineInput(fin_states=np.array([0, 0, 0, 0]), airbrake_angle=0.0))

while not env.finished():
    recv_data = physx_sock.receive()

    if recv_data is None:
        print("\n> Simulation ended unexpectedly (no data received from physics engine)!")
        exit(0)

    data = datalink.sitl_response_data.unpack(recv_data)
    received_data.append(data)

    env.forward(PhysicsEngineInput(fin_states=np.array([data.angle_fin1, data.angle_fin2, data.angle_fin3, data.angle_fin4]), airbrake_angle=data.angle_airbrake))
    true_data.append(env.get_current_true_data())

    physx_sock.send(env.get_current_data().pack())

    if tick % 100 == 0:
        print(f"\r> Running simulation... Time: {tick/1000:.1f}s (Real: {time.time() - start_time:.1f}s)", end="", flush=True)

    tick += 1

print(f"\n> Simulation complete (in {time.time() - start_time:.1f}s)!")
# ============================================


# ================= CLEANUP =================
physx_sock.close()
radio_sock.close()
# ============================================


# ================== PLOTTING ==================
print("> Plotting data...")

# main_plot.plot(received_data, SIM_TICK_DT)
# axes_plots.plot(received_data, true_data, SIM_TICK_DT)
# errors_plots.plot(received_data, true_data, SIM_TICK_DT)
# predicted_apogee_plot.plot(received_data, SIM_TICK_DT)

# plt.pause(0.1)
# plt.show()

attitude_anim.animate_quaternions(received_data, true_data, SIM_TICK_DT, target_fps=30, speedup_factor=1.0)
# ==============================================
