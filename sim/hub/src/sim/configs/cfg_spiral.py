from sim.env.environments import *
from sim.env.physics_engines import *
import numpy as np


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
        start_time: float = 0.0,
        max_time: float | None = None,
        relative_to_first: bool = True,
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
        # If True, returned positions will be offset so the first sample is the origin
        self.relative_to_first = relative_to_first
        self._first_true_pos = None

        self.acc = np.zeros(3)
        self.vel = np.zeros(3)
        self.pos = self.center.copy()
        self.w = np.array([0.0, 0.0, 0.0])
        self.q = np.array([1.0, 0.0, 0.0, 0.0])

        self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=self.pos, w=self.w, q=self.q)
        self.physics_time = 0.0
        self.start_time = start_time
        self.max_time = max_time if max_time is not None else 60.0

    def integrate(self, input: PhysicsEngineInput) -> PhysicsEngineOutput:
        if self.time > self.start_time:
            self.physics_time += self.dt

            orbit_angle = self.orbit_omega * self.physics_time + self.orbit_phase
            vertical_angle = self.vertical_omega * self.physics_time + self.vertical_phase
            spin_angle = self.spin_w * self.physics_time + self.spin_phase

            cos_orbit = np.cos(orbit_angle)
            sin_orbit = np.sin(orbit_angle)
            cos_vertical = np.cos(vertical_angle)
            sin_vertical = np.sin(vertical_angle)

            # NED frame: [north, east, down]
            north = self.radius * cos_orbit
            east = self.radius * sin_orbit
            down = self.vertical_amplitude * sin_vertical
            self.pos = self.center + np.array([north, east, down])

            vel_north = -self.radius * self.orbit_omega * sin_orbit
            vel_east = self.radius * self.orbit_omega * cos_orbit
            vel_down = self.vertical_amplitude * self.vertical_omega * cos_vertical
            self.vel = np.array([vel_north, vel_east, vel_down])

            acc_north = -self.radius * self.orbit_omega**2 * cos_orbit
            acc_east = -self.radius * self.orbit_omega**2 * sin_orbit
            acc_down = -self.vertical_amplitude * self.vertical_omega**2 * sin_vertical
            self.acc = np.array([acc_north, acc_east, acc_down])

            # FRD body rates: positive wz is rotation about body down axis.
            self.w = np.array([0.0, 0.0, self.spin_w])
            # Quaternion is FRD -> NED, representing yaw about +Down axis.
            self.q = np.array([np.cos(0.5 * spin_angle), 0.0, 0.0, np.sin(0.5 * spin_angle)])

            # If configured, make the returned position relative to the first true sample
            if self.relative_to_first:
                if self._first_true_pos is None:
                    self._first_true_pos = self.pos.copy()

                ret_pos = self.pos - self._first_true_pos
            else:
                ret_pos = self.pos

            self.current_state = PhysicsEngineOutput(acc=self.acc, vel=self.vel, pos=ret_pos, w=self.w, q=self.q)

        self.time += self.dt

        return self.current_state

    def finished(self) -> bool:
        return self.time >= self.max_time


def get_environment(dt: float):
    return SyntheticEnvironment(
        engine=CircularSpiralPhysicsEngine(dt=dt, start_time=2.0, max_time=30.0),
        imu1=SyntheticIMUModel(rate=500, noise_acc=GaussianNoiseModel(mean=0, std_dev=0.055), noise_gyro=GaussianNoiseModel(mean=0, std_dev=0.17), acc_range_g=6.0, gyro_range_deg=500.0),
        mag1=SyntheticMagnetometerModel(rate=100, noise=GaussianNoiseModel(mean=0, std_dev=0.0004)),
        baro1=SyntheticBarometerModel(rate=50, noise=GaussianNoiseModel(mean=0, std_dev=0.4)),
        gps1=SyntheticGPSModel(rate=25, noise_hor=GaussianNoiseModel(mean=0, std_dev=1.7), noise_ver=GaussianNoiseModel(mean=0, std_dev=3.1), noise_vel=GaussianNoiseModel(mean=0, std_dev=0.5), lat=50.337497, lon=19.525838, alt=30),
    )
