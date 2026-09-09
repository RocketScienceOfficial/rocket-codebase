from sim.env.environments import *
from sim.env.physics_engines import *
from sim.env.registry import build_sensors


class StandingBodyPhysicsEngine(PhysicsEngineInterface):
    def __init__(self, dt: float, max_time: float | None = None):
        super().__init__(dt)

        self.acc = np.array([0.0, 0.0, 0.0])
        self.vel = np.array([0.0, 0.0, 0.0])
        self.pos = np.array([0.0, 0.0, 0.0])
        self.w = np.array([0.0, 0.0, 0.0])
        self.q = np.array([1.0, 0.0, 0.0, 0.0])

        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)
        self.max_time = max_time if max_time is not None else 60.0

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        self.time += self.dt

        return self.current_state

    def finished(self) -> bool:
        return self.time >= self.max_time


def get_environment(dt: float, board: str | None = None):
    sensors = build_sensors(board)

    return SyntheticEnvironment(
        engine=StandingBodyPhysicsEngine(dt=dt, max_time=30.0),
        imu1=sensors.imu1,
        mag1=sensors.mag1,
        baro1=sensors.baro1,
        gps1=sensors.gps1,
    )
