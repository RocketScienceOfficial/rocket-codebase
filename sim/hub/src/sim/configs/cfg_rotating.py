from sim.env.environments import *
from sim.env.physics_engines import *


class RotatingBodyPhysicsEngine(PhysicsEngineInterface):
    def __init__(self, dt: float, speed: float | None = None, max_time: float | None = None):
        super().__init__(dt)

        self.acc = np.array([0.0, 0.0, 0.0])
        self.vel = np.array([0.0, 0.0, 0.0])
        self.pos = np.array([0.0, 0.0, 0.0])
        self.w = np.array([0.0, 0.0, speed if speed is not None else 0.5])
        self.q = np.array([1.0, 0.0, 0.0, 0.0])

        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)
        self.max_time = max_time if max_time is not None else 60.0

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
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


def get_environment(dt: float):
    return SyntheticEnvironment(
        engine=RotatingBodyPhysicsEngine(dt=dt, speed=5.0, max_time=30.0),
        imu1=SyntheticIMUModel(rate=500, noise_acc=GaussianNoiseModel(mean=0, std_dev=0.055), noise_gyro=GaussianNoiseModel(mean=0, std_dev=0.17), acc_range_g=6.0, gyro_range=500.0),
        mag1=SyntheticMagnetometerModel(rate=100, noise=GaussianNoiseModel(mean=0, std_dev=0.0004)),
        baro1=SyntheticBarometerModel(rate=50, noise=GaussianNoiseModel(mean=0, std_dev=0.4)),
        gps1=SyntheticGPSModel(rate=25, noise_hor=GaussianNoiseModel(mean=0, std_dev=1.7), noise_ver=GaussianNoiseModel(mean=0, std_dev=3.1), noise_vel=GaussianNoiseModel(mean=0, std_dev=0.5), lat=50.337497, lon=19.525838, alt=30),
    )
