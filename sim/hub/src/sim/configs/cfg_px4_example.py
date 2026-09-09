from sim.env.environments import PX4ReplayEnvironment


def get_environment(dt: float, board: str | None = None):
    if board is not None:
        raise ValueError("cfg_px4_example is a replay scenario and does not use a board")

    # Got from: https://logs.px4.io/browse
    return PX4ReplayEnvironment(
        ulog_path="./data/px4_example.ulg",
        imu_rate=500,
        dt=dt,
    )
