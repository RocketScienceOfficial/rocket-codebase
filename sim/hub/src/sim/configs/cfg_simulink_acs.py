from sim.env.environments import *
from sim.env.physics_engines import *
from sim.env.registry import build_sensors


def get_environment(dt: float, board: str | None = None):
    sensors = build_sensors(board)

    return SyntheticEnvironment(
        engine=SimulinkPhysicsEngine(dt=dt, model="acs"),
        imu1=sensors.imu1,
        mag1=sensors.mag1,
        baro1=sensors.baro1,
        gps1=sensors.gps1,
    )
