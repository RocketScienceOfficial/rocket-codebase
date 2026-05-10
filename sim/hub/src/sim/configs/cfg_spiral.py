from sim.env.environments import *
from sim.env.physics_engines import *


class CircularSpiralPhysicsEngine(PhysicsEngineInterface):
    def __init__(
        self,
        dt: float,
        radius: float = 10.0,
        orbit_period: float = 20.0,
        vertical_amplitude: float = 1.0,
        vertical_period: float = 10.0,
        w: float = 0.5,
        center: np.ndarray | None = None,
        orbit_phase: float = 0.0,
        vertical_phase: float = 0.0,
        spin_phase: float = 0.0,
        max_time: float | None = None,
    ):
        super().__init__(dt)

        self.radius = radius
        self.orbit_omega = 0.0 if orbit_period <= 0.0 else 2.0 * np.pi / orbit_period
        self.vertical_amplitude = vertical_amplitude
        self.vertical_omega = 0.0 if vertical_period <= 0.0 else 2.0 * np.pi / vertical_period
        self.spin_w = w
        self.center = np.zeros(3) if center is None else np.asarray(center, dtype=float)
        self.orbit_phase = orbit_phase
        self.vertical_phase = vertical_phase
        self.spin_phase = spin_phase

        self.acc = np.zeros(3)
        self.vel = np.zeros(3)
        self.pos = self.center.copy()
        self.w = np.array([0.0, 0.0, self.spin_w])
        self.q = np.array([1.0, 0.0, 0.0, 0.0])

        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)
        self.max_time = max_time if max_time is not None else 60.0

    def _state_at_time(self, time: float) -> PhysicsEngineOutput:
        orbit_angle = self.orbit_omega * time + self.orbit_phase
        vertical_angle = self.vertical_omega * time + self.vertical_phase
        spin_angle = self.spin_w * time + self.spin_phase

        cos_orbit = np.cos(orbit_angle)
        sin_orbit = np.sin(orbit_angle)
        cos_vertical = np.cos(vertical_angle)
        sin_vertical = np.sin(vertical_angle)

        self.pos = self.center + np.array([
            self.radius * cos_orbit,
            self.radius * sin_orbit,
            self.vertical_amplitude * sin_vertical,
        ])

        self.vel = np.array([
            -self.radius * self.orbit_omega * sin_orbit,
            self.radius * self.orbit_omega * cos_orbit,
            self.vertical_amplitude * self.vertical_omega * cos_vertical,
        ])

        self.acc = np.array([
            -self.radius * self.orbit_omega**2 * cos_orbit,
            -self.radius * self.orbit_omega**2 * sin_orbit,
            -self.vertical_amplitude * self.vertical_omega**2 * sin_vertical,
        ])

        self.w = np.array([0.0, 0.0, self.spin_w])
        self.q = np.array([np.cos(0.5 * spin_angle), 0.0, 0.0, np.sin(0.5 * spin_angle)])

        return PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        self.time += self.dt
        self.current_state = self._state_at_time(self.time)

        return self.current_state

    def finished(self) -> bool:
        return self.time >= self.max_time


def get_environment(dt: float):
    return SyntheticEnvironment(
        engine=CircularSpiralPhysicsEngine(dt=dt),
        imu1=SyntheticIMUModel(rate=500, noise_acc=GaussianNoiseModel(mean=0, std_dev=0.055), noise_gyro=GaussianNoiseModel(mean=0, std_dev=0.17), acc_range_g=6.0, gyro_range=500.0),
        mag1=SyntheticMagnetometerModel(rate=100, noise=GaussianNoiseModel(mean=0, std_dev=0.0004)),
        baro1=SyntheticBarometerModel(rate=50, noise=GaussianNoiseModel(mean=0, std_dev=0.4)),
        gps1=SyntheticGPSModel(rate=25, noise_hor=GaussianNoiseModel(mean=0, std_dev=1.7), noise_ver=GaussianNoiseModel(mean=0, std_dev=3.1), noise_vel=GaussianNoiseModel(mean=0, std_dev=0.5), lat=50.337497, lon=19.525838, alt=30),
    )
