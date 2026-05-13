from sim.utils import sensor_utils
from .physics_engines import *


class NoiseModelInterface:
    def get_noise(self, shape) -> np.ndarray:
        pass

    def get_standard_deviation(self):
        pass


class GaussianNoiseModel(NoiseModelInterface):
    def __init__(self, mean, std_dev):
        self.mean = mean
        self.std_dev = std_dev

    def get_noise(self, shape) -> np.ndarray:
        return np.random.normal(self.mean, self.std_dev, shape)

    def get_standard_deviation(self):
        return self.std_dev


@dataclass
class IMUModelOutput:
    acc: np.ndarray
    gyro: np.ndarray
    clipping_flags: np.ndarray
    dt: float


@dataclass
class MagnetometerModelOutput:
    mag: np.ndarray


@dataclass
class BarometerModelOutput:
    pressure: int
    temperature: float
    height: float


@dataclass
class GPSModelOutput:
    pos: np.ndarray
    vel: np.ndarray
    stddevs: np.ndarray
    sats: int


class SensorModelInterface:
    def __init__(self, rate: int):
        self.rate = rate
        self.last_update_time = 0.0

    def is_update_time(self, time):
        return time - self.last_update_time >= (1.0 / self.rate) - 1e-6

    def get_measurement(self, time: float):
        self.last_update_time = time


class IMUModelInterface(SensorModelInterface):
    def get_measurement(self, time: float) -> IMUModelOutput:
        super().get_measurement(time)


class SyntheticIMUModel(IMUModelInterface):
    def __init__(self, rate: int, noise_acc: NoiseModelInterface, noise_gyro: NoiseModelInterface, acc_range_g: float, gyro_range_deg: float):
        super().__init__(rate)

        self.noise_acc = noise_acc
        self.noise_gyro = noise_gyro
        self.acc_range = acc_range_g * geo.g
        self.gyro_range = gyro_range_deg * np.pi / 180.0

    def set_state(self, state: PhysicsEngineOutput):
        self.state = state

    def get_measurement(self, time: float) -> IMUModelOutput:
        super().get_measurement(time)

        acc_meas_ned = self.state.acc - geo.g_ned
        q_ned_to_frd = quat.quat_conj(self.state.q)
        acc_body = quat.quat_rotate_vector(q_ned_to_frd, acc_meas_ned)
        acc_body_meas = acc_body + self.noise_acc.get_noise(3)
        gyro_meas = self.state.w + self.noise_gyro.get_noise(3)

        clipping_flags = sensor_utils.get_imu_clipping_flags(acc_body_meas, gyro_meas, self.acc_range, self.gyro_range)
        acc_body_meas = np.clip(acc_body_meas, -self.acc_range, self.acc_range)
        gyro_meas = np.clip(gyro_meas, -self.gyro_range, self.gyro_range)
        dt = 1.0 / self.rate

        return IMUModelOutput(acc=acc_body_meas, gyro=gyro_meas, clipping_flags=clipping_flags, dt=dt)


class ReplayIMUModel(IMUModelInterface):
    def __init__(self, rate: int, acc_x: str, acc_y: str, acc_z: str, gyro_x: str, gyro_y: str, gyro_z: str, acc_range_g: float, gyro_range_deg: float):
        super().__init__(rate)

        self.cols = [acc_x, acc_y, acc_z, gyro_x, gyro_y, gyro_z]
        self.clipping_acc_range = acc_range_g * geo.g
        self.clipping_gyro_range = gyro_range_deg * np.pi / 180.0

    def set_df(self, times: np.ndarray, df: pd.DataFrame):
        if not all(col in df.columns for col in self.cols):
            raise ValueError("One or more IMU columns not found in dataframe")

        self.times = times
        self.data = df[self.cols].values

    def get_measurement(self, time: float) -> IMUModelOutput:
        super().get_measurement(time)

        acc_meas = np.array(sensor_utils.get_values_interp(time, self.times, self.data[:, 0:3]))
        gyro_meas = np.array(sensor_utils.get_values_interp(time, self.times, self.data[:, 3:6]))
        clipping_flags = sensor_utils.get_imu_clipping_flags(acc_meas, gyro_meas, self.clipping_acc_range, self.clipping_gyro_range)

        return IMUModelOutput(acc=acc_meas, gyro=gyro_meas, clipping_flags=clipping_flags, dt=1.0/self.rate)


class MagnetometerModelInterface(SensorModelInterface):
    def get_measurement(self, time: float) -> MagnetometerModelOutput:
        super().get_measurement(time)


class SyntheticMagnetometerModel(MagnetometerModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface, field_ned: np.ndarray | None = None):
        super().__init__(rate)

        self.noise = noise
        self.field_ned = geo.mag_ned if field_ned is None else np.asarray(field_ned, dtype=float)
        self.buffer = sensor_utils.SensorDelayedBuffer(delay_ms=10, initial_value=MagnetometerModelOutput(mag=self.field_ned))

    def set_state(self, state: PhysicsEngineOutput):
        self.state = state

    def get_measurement(self, time: float) -> MagnetometerModelOutput:
        super().get_measurement(time)

        q_ned_to_frd = quat.quat_conj(self.state.q)
        b_body = quat.quat_rotate_vector(q_ned_to_frd, self.field_ned)
        mag = b_body + self.noise.get_noise(3)

        meas = self.buffer.update(time, MagnetometerModelOutput(mag=mag))

        return meas


class ReplayMagnetometerModel(MagnetometerModelInterface):
    def __init__(self, rate: int, mag_x: str, mag_y: str, mag_z: str, scale: float = 1.0):
        super().__init__(rate)

        self.mag_x = mag_x
        self.mag_y = mag_y
        self.mag_z = mag_z
        self.scale = scale

    def set_df(self, times: np.ndarray, df: pd.DataFrame):
        if not all(col in df.columns for col in [self.mag_x, self.mag_y, self.mag_z]):
            raise ValueError("One or more Magnetometer columns not found in dataframe")

        self.times = times
        self.data = df[[self.mag_x, self.mag_y, self.mag_z]].values

    def get_measurement(self, time: float) -> MagnetometerModelOutput:
        super().get_measurement(time)

        mag_meas = np.array(sensor_utils.get_values_interp(time, self.times, self.data)) * self.scale

        return MagnetometerModelOutput(mag=mag_meas)


class GPSModelInterface(SensorModelInterface):
    def get_measurement(self, time: float) -> GPSModelOutput:
        super().get_measurement(time)


class SyntheticGPSModel(GPSModelInterface):
    def __init__(self, rate: int, noise_hor: NoiseModelInterface, noise_ver: NoiseModelInterface, noise_vel: NoiseModelInterface, lat: float, lon: float, alt: float):
        super().__init__(rate)

        self.noise_hor = noise_hor
        self.noise_ver = noise_ver
        self.noise_vel = noise_vel
        self.base_lat = lat
        self.base_lon = lon
        self.base_alt = alt
        self.default_sats = 14
        self.buffer = sensor_utils.SensorDelayedBuffer(delay_ms=100, initial_value=GPSModelOutput(pos=np.array([lat, lon, alt]), vel=np.zeros(3), stddevs=self._get_stddevs(), sats=self.default_sats))

    def set_state(self, state: PhysicsEngineOutput):
        self.state = state

    def get_measurement(self, time: float) -> GPSModelOutput:
        super().get_measurement(time)

        pos_noise = np.array([self.noise_hor.get_noise(1)[0], self.noise_hor.get_noise(1)[0], self.noise_ver.get_noise(1)[0]])
        vel_noise = self.noise_vel.get_noise(3)

        pos = self.state.pos + pos_noise
        vel = self.state.vel + vel_noise
        lat, lon, alt = geo.ned_to_geo(self.base_lat, self.base_lon, self.base_alt, pos[0], pos[1], pos[2])

        meas = self.buffer.update(time, GPSModelOutput(pos=np.array([lat, lon, alt]), vel=vel, stddevs=self._get_stddevs(), sats=self.default_sats))

        return meas

    def _get_stddevs(self):
        stddev_hor = self.noise_hor.get_standard_deviation()
        stddev_ver = self.noise_ver.get_standard_deviation()
        stddev_vel = self.noise_vel.get_standard_deviation()
        return np.array([stddev_hor, stddev_ver, stddev_vel])


class ReplayGPSModel(GPSModelInterface):
    def __init__(self, rate: int, lat: str, lon: str, alt: str, vel_n: str, vel_e: str, vel_d: str, sats: str | int | None, stddev_hor: str | float | None, stddev_ver: str | float | None, stddev_vel: str | float | None):
        super().__init__(rate)

        self.lat = lat
        self.lon = lon
        self.alt = alt
        self.vel_n = vel_n
        self.vel_e = vel_e
        self.vel_d = vel_d
        self.sats = sats
        self.stddev_hor = stddev_hor
        self.stddev_ver = stddev_ver
        self.stddev_vel = stddev_vel

    def set_df(self, times: np.ndarray, df: pd.DataFrame):
        if not all(col in df.columns for col in [self.lat, self.lon, self.alt]):
            raise ValueError("One or more GPS columns not found in dataframe")

        self.times = times
        self.pos_data = df[[self.lat, self.lon, self.alt]].values

        if all(col in df.columns for col in [self.vel_n, self.vel_e, self.vel_d]):
            self.vel_data = df[[self.vel_n, self.vel_e, self.vel_d]].values
        else:
            self.vel_data = None

        # Optional scalar/column inputs: sats and stddevs can be strings (column names), numbers, or None
        # If strings, ensure columns exist and store data arrays for interpolation
        def validate_optional_field(field_name):
            field_value = getattr(self, field_name)
            if isinstance(field_value, str):
                if field_value not in df.columns:
                    raise ValueError(f"{field_name} column '{field_value}' not found in dataframe")
                setattr(self, f"{field_name}_data", np.array([df[field_value].values]).T)  # Store as 2D array for interpolation
            else:
                setattr(self, f"{field_name}_data", None)

        validate_optional_field('sats')
        validate_optional_field('stddev_hor')
        validate_optional_field('stddev_ver')
        validate_optional_field('stddev_vel')

    def get_measurement(self, time: float) -> GPSModelOutput:
        super().get_measurement(time)

        gps_meas = sensor_utils.get_values_interp(time, self.times, self.pos_data)
        vel_meas = np.zeros(3)

        if self.vel_data is not None:
            vel_meas = sensor_utils.get_values_interp(time, self.times, self.vel_data)

        # stddevs: string -> interp, number -> use, None -> default
        def resolve_optional_field(field, default):
            val = getattr(self, field)
            if isinstance(val, str):
                return float(sensor_utils.get_values_interp(time, self.times, getattr(self, f"{field}_data"))[0])  # Interpolate and convert to float
            if isinstance(val, (int, float)):
                return float(val)
            return float(default)

        sats = int(resolve_optional_field('sats', 14))
        std_h = resolve_optional_field('stddev_hor', 1.7)
        std_v = resolve_optional_field('stddev_ver', 3.0)
        std_vel = resolve_optional_field('stddev_vel', 0.5)

        return GPSModelOutput(pos=gps_meas, vel=vel_meas, stddevs=np.array([std_h, std_v, std_vel]), sats=sats)


class EventReplayGPSModel(GPSModelInterface):
    def __init__(self):
        super().__init__(rate=1)
        self._times: np.ndarray | None = None
        self._data: np.ndarray | None = None
        self._cursor = 0

    def set_data(self, times: np.ndarray, data: np.ndarray) -> None:
        self._times = times
        self._data = data
        self._cursor = 0

    def is_update_time(self, time: float) -> bool:
        if self._times is None or self._cursor >= len(self._times):
            return False
        return time >= self._times[self._cursor]

    def get_measurement(self, time: float) -> GPSModelOutput:
        row = self._data[self._cursor]
        self._cursor += 1
        return GPSModelOutput(
            pos=np.array([row[0], row[1], row[2]]),
            vel=np.array([row[3], row[4], row[5]]),
            stddevs=np.array([row[6], row[7], row[8]]),
            sats=int(row[9]),
        )


class EventReplayMagnetometerModel(MagnetometerModelInterface):
    def __init__(self):
        super().__init__(rate=1)
        self._times: np.ndarray | None = None
        self._data: np.ndarray | None = None
        self._cursor = 0

    def set_data(self, times: np.ndarray, data: np.ndarray) -> None:
        self._times = times
        self._data = data
        self._cursor = 0

    def is_update_time(self, time: float) -> bool:
        if self._times is None or self._cursor >= len(self._times):
            return False
        return time >= self._times[self._cursor]

    def get_measurement(self, time: float) -> MagnetometerModelOutput:
        mag = self._data[self._cursor].copy()
        self._cursor += 1
        return MagnetometerModelOutput(mag=mag)


class BarometerModelInterface(SensorModelInterface):
    def get_measurement(self, time: float) -> BarometerModelOutput:
        super().get_measurement(time)


class SyntheticBarometerModel(BarometerModelInterface):
    def __init__(self, rate: int, noise: NoiseModelInterface):
        super().__init__(rate)

        self.noise = noise
        self.buffer = sensor_utils.SensorDelayedBuffer(delay_ms=20, initial_value=BarometerModelOutput(height=0.0, pressure=101325, temperature=15.0))

    def set_state(self, state: PhysicsEngineOutput):
        self.state = state

    def get_measurement(self, time: float) -> BarometerModelOutput:
        super().get_measurement(time)

        height = -self.state.pos[2]
        pressure = 101325 * (1 - height / 44330) ** 5.255
        temperature = 15 - height * 0.0065

        height = height + self.noise.get_noise(1)[0]
        pressure = int(pressure + self.noise.get_noise(1)[0])  # Pressure should be int
        temperature = temperature + self.noise.get_noise(1)[0]

        meas = self.buffer.update(time, BarometerModelOutput(height=height, pressure=pressure, temperature=temperature))

        return meas


class ReplayBarometerModel(BarometerModelInterface):
    def __init__(self, rate: int, pressure: str, temperature: str, height: str):
        super().__init__(rate)

        self.pressure = pressure
        self.temperature = temperature
        self.height = height

    def set_df(self, times: np.ndarray, df: pd.DataFrame):
        if not all(col in df.columns for col in [self.pressure, self.temperature, self.height]):
            raise ValueError("One or more Barometer columns not found in dataframe")

        self.times = times
        self.data = df[[self.pressure, self.temperature, self.height]].values

    def get_measurement(self, time: float) -> BarometerModelOutput:
        super().get_measurement(time)

        baro_meas = sensor_utils.get_values_interp(time, self.times, self.data)
        baro_meas[0] = int(baro_meas[0])  # Pressure should be int

        return BarometerModelOutput(pressure=baro_meas[0], temperature=baro_meas[1], height=baro_meas[2])


class EventReplayBarometerModel(BarometerModelInterface):
    def __init__(self):
        super().__init__(rate=1)
        self._times: np.ndarray | None = None
        self._data: np.ndarray | None = None  # shape (N, 3): [pressure, temperature, height]
        self._cursor = 0

    def set_data(self, times: np.ndarray, data: np.ndarray) -> None:
        """times: 1D seconds. data: shape (N, 3) — [pressure (Pa), temperature (°C), height (m)]."""
        self._times = times
        self._data = data
        self._cursor = 0

    def is_update_time(self, time: float) -> bool:
        if self._times is None or self._cursor >= len(self._times):
            return False
        return time >= self._times[self._cursor]

    def get_measurement(self, time: float) -> BarometerModelOutput:
        row = self._data[self._cursor]
        self._cursor += 1
        return BarometerModelOutput(pressure=int(row[0]), temperature=float(row[1]), height=float(row[2]))
