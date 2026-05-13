from sim.env.environments import PX4ReplayEnvironment


def get_environment(dt: float):
    # Got from: https://logs.px4.io/browse
    return PX4ReplayEnvironment(
        ulog_path="./data/px4_example.ulg",
        imu_rate=500,
        dt=dt,
    )
