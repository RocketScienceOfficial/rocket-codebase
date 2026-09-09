from sim.env.environments import *
from sim.env.physics_engines import *
from sim.env.registry import build_sensors


class RotatingBodyPhysicsEngine(PhysicsEngineInterface):
    def __init__(self, dt: float, speed: float | None = None, start_time: float = 0.0, max_time: float | None = None):
        super().__init__(dt)

        self.acc = np.array([0.0, 0.0, 0.0])
        self.vel = np.array([0.0, 0.0, 0.0])
        self.pos = np.array([0.0, 0.0, 0.0])
        self.w = np.array([0.0, 0.0, 0.0])
        self.q = np.array([1.0, 0.0, 0.0, 0.0])

        self.angular_speed = speed if speed is not None else 0.5
        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)
        self.start_time = start_time
        self.max_time = max_time if max_time is not None else 60.0

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        if self.time > self.start_time:
            self.w = np.array([0.0, 0.0, self.angular_speed])
            omega_q = np.array([0.0, self.w[0], self.w[1], self.w[2]])
            q_dot = 0.5 * quat.quat_multiply(self.q, omega_q)
            self.q = self.q + q_dot * self.dt
            n = np.linalg.norm(self.q)

            if n > 0.0:
                self.q = self.q / n

        self.time += self.dt
        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)

        return self.current_state

    def finished(self) -> bool:
        return self.time >= self.max_time


def get_environment(dt: float, board: str | None = None):
    sensors = build_sensors(board)

    return SyntheticEnvironment(
        engine=RotatingBodyPhysicsEngine(dt=dt, speed=0.5, start_time=2.0, max_time=30.0),
        imu1=sensors.imu1,
        mag1=sensors.mag1,
        baro1=sensors.baro1,
        gps1=sensors.gps1,
    )
