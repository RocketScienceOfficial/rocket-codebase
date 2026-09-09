from sim.env.environments import *
from sim.env.physics_engines import *
from sim.env.registry import build_sensors
from sim.utils.geo import g


class ColdflowSimScenario(SimpleIntegratorScenarioInterface):
    def __init__(self, max_time: float | None = None):
        self.time = 0.0
        self.max_time = max_time if max_time is not None else 10.0

    def get_net_acc(self, time: float, current_state: PhysicsEngineOutput, input: PhysicsEngineInput) -> np.ndarray:
        self.time = time

        if time < 1.5:
            return np.array([0.0, 0.0, -1.0 * g])  # Should trigger altitude fallback
        else:
            return np.array([0.0, 0.0, g])

    def finished(self) -> bool:
        return self.time >= self.max_time


def get_environment(dt: float, board: str | None = None):
    sensors = build_sensors(board)

    return SyntheticEnvironment(
        engine=SimpleIntegratorPhysicsEngine(dt=dt, scenario=ColdflowSimScenario(max_time=4.0), init_time=2.0),
        imu1=sensors.imu1,
        mag1=sensors.mag1,
        baro1=sensors.baro1,
        gps1=sensors.gps1,
    )
