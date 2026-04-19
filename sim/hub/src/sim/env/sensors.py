import numpy as np
from .physics_engines import *
from sim.utils import quat, geo


class NoiseModelInterface:
    def get_noise(self, shape) -> np.ndarray:
        pass


class GaussianNoiseModel(NoiseModelInterface):
    def __init__(self, mean, var):
        self.mean = mean
        self.var = var

    def get_noise(self, shape) -> np.ndarray:
        return np.random.normal(self.mean, self.var**0.5, shape)


class SensorModelInterface:
    def __init__(self, rate: int, noise: NoiseModelInterface):
        self.rate = rate
        self.noise = noise
        self.last_update_time = 0.0

    def is_update_time(self, time):
        return time - self.last_update_time >= (1.0 / self.rate) - 1e-6

    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        self.last_update_time = time


class AccelerometerModel(SensorModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface):
        super().__init__(rate, noise)

    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        super().get_measurement(time, state)

        f_ned = state.acc - G_STD
        f_body = quat.ned_to_body(state.q, f_ned)
        meas = f_body + self.noise.get_noise(3)

        return meas


class GyroscopeModel(SensorModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface):
        super().__init__(rate, noise)

    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        super().get_measurement(time, state)

        meas = state.w + self.noise.get_noise(3)

        return meas


class IMUModel(SensorModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface):
        super().__init__(rate, noise)

        self.acc = AccelerometerModel(rate, noise)
        self.gyro = GyroscopeModel(rate, noise)

    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        super().get_measurement(time, state)

        acc_meas = self.acc.get_measurement(time, state)
        gyro_meas = self.gyro.get_measurement(time, state)
        dt = 1.0 / self.rate

        return acc_meas, gyro_meas, dt


class MagnetometerModel(SensorModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface, field_ned: np.ndarray | None = None):
        super().__init__(rate, noise)

        self.field_ned = MAG_FIELD_NED if field_ned is None else np.asarray(field_ned, dtype=float)

    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        super().get_measurement(time, state)

        b_body = quat.ned_to_body(state.q, self.field_ned)
        meas = b_body + self.noise.get_noise(3)

        return meas


class GPSModel(SensorModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface, lat: float, lon: float, alt: float):
        super().__init__(rate, noise)

        self.base_lat = lat
        self.base_lon = lon
        self.base_alt = alt

    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        super().get_measurement(time, state)

        pos = state.pos + self.noise.get_noise(3)
        lat, lon, alt = geo.ned_to_geo(self.base_lat, self.base_lon, self.base_alt, pos[0], pos[1], pos[2])
        sats = 14

        return np.array([lat, lon, alt]), sats


class BarometerModel(SensorModelInterface):
    def get_measurement(self, time: float, state: PhysicsEngineOutput):
        super().get_measurement(time, state)

        height = state.pos[2]
        pressure = 101325 * (1 - height / 44330) ** 5.255
        temperature = 15 - height * 0.0065

        height = height + self.noise.get_noise(1)[0]
        pressure = int(pressure + self.noise.get_noise(1)[0])
        temperature = temperature + self.noise.get_noise(1)[0]

        return height, pressure, temperature
