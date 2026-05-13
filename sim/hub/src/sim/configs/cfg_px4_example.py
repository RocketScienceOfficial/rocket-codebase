from sim.env.environments import PX4ReplayEnvironment


def get_environment(dt: float):
    return PX4ReplayEnvironment(
        ulog_path="./data/px4_example.ulg",
        imu_rate=500,
        dt=dt,
    )
