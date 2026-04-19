from sim import datalink
from .physics_engines import *
from .sensors import *
from dataclasses import dataclass
import pandas as pd
import numpy as np


class EnvironmentInterface:
    def __init__(self):
        self.current_data = datalink.sitl_request_data(
            readFlags=0,
            imu1dt=0,
            imu1AccX=0,
            imu1AccY=0,
            imu1AccZ=0,
            imu1GyroX=0,
            imu1GyroY=0,
            imu1GyroZ=0,
            mag1X=0,
            mag1Y=0,
            mag1Z=0,
            baro1Pressure=0,
            baro1Temperature=0,
            baro1Height=0,
            gps1Lat=0,
            gps1Lon=0,
            gps1Alt=0,
            gps1Sats=0,
        )

    def forward(self, input: PhysicsEngineInput) -> None:
        pass

    def get_current_data(self) -> datalink.sitl_request_data:
        return self.current_data

    def get_current_true_data(self) -> PhysicsEngineOutput:
        return None

    def finished(self) -> bool:
        pass


class SyntheticEnvironment(EnvironmentInterface):
    def __init__(self, engine: PhysicsEngineInterface, imu1: IMUModel, mag1: MagnetometerModel, baro1: BarometerModel, gps1: GPSModel):
        super().__init__()

        self.engine = engine
        self.current_state = None
        self.imu1 = imu1
        self.mag1 = mag1
        self.baro1 = baro1
        self.gps1 = gps1

    def forward(self, input: PhysicsEngineInput) -> None:
        state = self.engine.integrate(input)

        if state is None:
            return

        self.current_state = state
        self.current_data.readFlags = 0

        if self.imu1.is_update_time(self.engine.time):
            acc_meas, gyro_meas, dt = self.imu1.get_measurement(self.engine.time, state)

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_IMU_1
            self.current_data.imu1dt = dt
            self.current_data.imu1AccX = acc_meas[0]
            self.current_data.imu1AccY = acc_meas[1]
            self.current_data.imu1AccZ = acc_meas[2]
            self.current_data.imu1GyroX = gyro_meas[0]
            self.current_data.imu1GyroY = gyro_meas[1]
            self.current_data.imu1GyroZ = gyro_meas[2]
        if self.mag1.is_update_time(self.engine.time):
            mag_meas = self.mag1.get_measurement(self.engine.time, state)

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_MAG_1
            self.current_data.mag1X = mag_meas[0]
            self.current_data.mag1Y = mag_meas[1]
            self.current_data.mag1Z = mag_meas[2]
        if self.baro1.is_update_time(self.engine.time):
            baro_meas = self.baro1.get_measurement(self.engine.time, state)

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_BARO_1
            self.current_data.baro1Height = baro_meas[0]
            self.current_data.baro1Pressure = baro_meas[1]
            self.current_data.baro1Temperature = baro_meas[2]
        if self.gps1.is_update_time(self.engine.time):
            gps_meas, gps_sats = self.gps1.get_measurement(self.engine.time, state)

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_GPS_1
            self.current_data.gps1Lat = gps_meas[0]
            self.current_data.gps1Lon = gps_meas[1]
            self.current_data.gps1Alt = gps_meas[2]
            self.current_data.gps1Sats = gps_sats

    def get_current_true_data(self) -> PhysicsEngineOutput:
        return self.current_state

    def finished(self) -> bool:
        return self.engine.finished()


class ReplaySensorTimer:
    def __init__(self, dt: float):
        self.dt = dt
        self.last_update_time = None

    def check_update(self, time: float) -> bool:
        if self.last_update_time is None:
            self.last_update_time = 0.0

        if time - self.last_update_time >= self.dt - 1e-6:
            self.last_update_time = time
            return True
        else:
            return False


@dataclass
class ReplayIMUData:
    dt: float
    acc_x: str
    acc_y: str
    acc_z: str
    gyro_x: str
    gyro_y: str
    gyro_z: str


@dataclass
class ReplayMagnetometerData:
    dt: float
    mag_x: str
    mag_y: str
    mag_z: str


@dataclass
class ReplayBarometerData:
    dt: float
    pressure: str
    temperature: str
    height: str


@dataclass
class ReplayGPSData:
    dt: float
    lat: str
    lon: str
    alt: str
    sats: str


class SequentialReplayEnvironment(EnvironmentInterface):
    def __init__(self, resample_dt: float, file_path: str, time_col: str, imu1: ReplayIMUData, mag1: ReplayMagnetometerData, baro1: ReplayBarometerData, gps1: ReplayGPSData):
        super().__init__()

        df = pd.read_csv(file_path)

        self.times = ((df[time_col] - df[time_col].iloc[0]) / 1e6).values
        self.vals = df[[imu1.acc_x, imu1.acc_y, imu1.acc_z, imu1.gyro_x, imu1.gyro_y, imu1.gyro_z, mag1.mag_x, mag1.mag_y, mag1.mag_z, baro1.pressure, baro1.temperature, baro1.height, gps1.lat, gps1.lon, gps1.alt, gps1.sats]].values

        self.imu1 = ReplaySensorTimer(imu1.dt)
        self.mag1 = ReplaySensorTimer(mag1.dt)
        self.baro1 = ReplaySensorTimer(baro1.dt)
        self.gps1 = ReplaySensorTimer(gps1.dt)

        self.time = 0.0
        self.dt = resample_dt

    def _get_values_interp(self, time, arr):
        new_data = []

        for col in range(arr.shape[1]):
            new_data.append(np.interp(time, self.times, arr[:, col]))

        return new_data

    def forward(self, input: PhysicsEngineInput) -> None:
        self.time += self.dt

        self.current_data.readFlags = 0

        if self.imu1.check_update(self.time):
            values = self._get_values_interp(self.time, self.vals[:, 0:6])

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_IMU_1
            self.current_data.imu1dt = self.dt
            self.current_data.imu1AccX = values[0]
            self.current_data.imu1AccY = values[1]
            self.current_data.imu1AccZ = values[2]
            self.current_data.imu1GyroX = values[3]
            self.current_data.imu1GyroY = values[4]
            self.current_data.imu1GyroZ = values[5]
        if self.mag1.check_update(self.time):
            values = self._get_values_interp(self.time, self.vals[:, 6:9])

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_MAG_1
            self.current_data.mag1X = values[0]
            self.current_data.mag1Y = values[1]
            self.current_data.mag1Z = values[2]
        if self.baro1.check_update(self.time):
            values = self._get_values_interp(self.time, self.vals[:, 9:12])

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_BARO_1
            self.current_data.baro1Pressure = int(values[0])
            self.current_data.baro1Temperature = values[1]
            self.current_data.baro1Height = values[2]
        if self.gps1.check_update(self.time):
            values = self._get_values_interp(self.time, self.vals[:, 12:16])

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_GPS_1
            self.current_data.gps1Lat = values[0]
            self.current_data.gps1Lon = values[1]
            self.current_data.gps1Alt = values[2]
            self.current_data.gps1Sats = int(values[3])

    def finished(self) -> bool:
        return self.time >= self.times[-1]
