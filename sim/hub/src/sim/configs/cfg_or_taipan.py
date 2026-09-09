from sim.env.environments import *
from sim.env.physics_engines import *
from sim.env.registry import build_sensors


def get_environment(dt: float, board: str | None = None):
    sensors = build_sensors(board)

    return SyntheticEnvironment(
        engine=SimpleIntegratorPhysicsEngine(dt=dt, scenario=OpenRocketSimScenario(file_path="./data/OR_TAIPAN.csv", preferred_time=30.0), init_time=2.0),
        imu1=sensors.imu1,
        mag1=sensors.mag1,
        baro1=sensors.baro1,
        gps1=sensors.gps1,
    )
