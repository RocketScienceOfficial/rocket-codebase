import pandas as pd
from sim import datalink
from .physics_engines import *
from .sensors import *
from sim.utils.ulog_loader import *


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
            imu1ClippingFlags=0,
            mag1X=0,
            mag1Y=0,
            mag1Z=0,
            baro1Pressure=0,
            baro1Temperature=0,
            baro1Height=0,
            gps1Lat=0,
            gps1Lon=0,
            gps1Alt=0,
            gps1VelN=0,
            gps1VelE=0,
            gps1VelD=0,
            gps1stddevHorizontal=0,
            gps1stddevVertical=0,
            gps1stddevSpeed=0,
            gps1Sats=0,
        )

        self.imu1 = IMUModelInterface(0)
        self.mag1 = MagnetometerModelInterface(0)
        self.baro1 = BarometerModelInterface(0)
        self.gps1 = GPSModelInterface(0)

    def update(self, input: PhysicsEngineInput) -> bool:
        pass

    def get_time(self) -> float:
        pass

    def forward(self, input: PhysicsEngineInput) -> None:
        if not self.update(input):
            return

        self.current_data.readFlags = 0

        if self.imu1.is_update_time(self.get_time()):
            meas = self.imu1.get_measurement(self.get_time())

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_IMU_1
            self.current_data.imu1dt = meas.dt
            self.current_data.imu1AccX = meas.acc[0]
            self.current_data.imu1AccY = meas.acc[1]
            self.current_data.imu1AccZ = meas.acc[2]
            self.current_data.imu1GyroX = meas.gyro[0]
            self.current_data.imu1GyroY = meas.gyro[1]
            self.current_data.imu1GyroZ = meas.gyro[2]
            self.current_data.imu1ClippingFlags = meas.clipping_flags
        if self.mag1.is_update_time(self.get_time()):
            meas = self.mag1.get_measurement(self.get_time())

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_MAG_1
            self.current_data.mag1X = meas.mag[0]
            self.current_data.mag1Y = meas.mag[1]
            self.current_data.mag1Z = meas.mag[2]
        if self.baro1.is_update_time(self.get_time()):
            meas = self.baro1.get_measurement(self.get_time())

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_BARO_1
            self.current_data.baro1Height = meas.height
            self.current_data.baro1Pressure = meas.pressure
            self.current_data.baro1Temperature = meas.temperature
        if self.gps1.is_update_time(self.get_time()):
            meas = self.gps1.get_measurement(self.get_time())

            self.current_data.readFlags |= datalink.sitl_read_flags.DATALINK_FLAGS_SITL_READ_GPS_1
            self.current_data.gps1Lat = meas.pos[0]
            self.current_data.gps1Lon = meas.pos[1]
            self.current_data.gps1Alt = meas.pos[2]
            self.current_data.gps1VelN = meas.vel[0]
            self.current_data.gps1VelE = meas.vel[1]
            self.current_data.gps1VelD = meas.vel[2]
            self.current_data.gps1stddevHorizontal = meas.stddevs[0]
            self.current_data.gps1stddevVertical = meas.stddevs[1]
            self.current_data.gps1stddevSpeed = meas.stddevs[2]
            self.current_data.gps1Sats = meas.sats

    def get_current_data(self) -> datalink.sitl_request_data:
        return self.current_data

    def get_current_true_data(self) -> PhysicsEngineOutput:
        pass

    def finished(self) -> bool:
        pass


class SyntheticEnvironment(EnvironmentInterface):
    def __init__(self, engine: PhysicsEngineInterface, imu1: SyntheticIMUModel, mag1: SyntheticMagnetometerModel, baro1: SyntheticBarometerModel, gps1: SyntheticGPSModel):
        super().__init__()

        self.engine = engine
        self.current_state = None
        self.imu1 = imu1
        self.mag1 = mag1
        self.baro1 = baro1
        self.gps1 = gps1

    def update(self, input: PhysicsEngineInput) -> bool:
        state = self.engine.integrate(input)

        if state is None:
            return False

        self.current_state = state

        self.imu1.set_state(state)
        self.mag1.set_state(state)
        self.baro1.set_state(state)
        self.gps1.set_state(state)

        return True

    def get_time(self) -> float:
        return self.engine.time

    def get_current_true_data(self) -> PhysicsEngineOutput:
        return self.current_state

    def finished(self) -> bool:
        return self.engine.finished()


class SequentialReplayEnvironment(EnvironmentInterface):
    def __init__(self, resample_dt: float, file_path: str, time_col: str, imu1: ReplayIMUModel, mag1: ReplayMagnetometerModel, baro1: ReplayBarometerModel, gps1: ReplayGPSModel):
        super().__init__()

        df = pd.read_csv(file_path)

        self.times = ((df[time_col] - df[time_col].iloc[0]) / 1e6).values
        self.time = 0.0
        self.dt = resample_dt

        self.imu1 = imu1
        self.imu1.set_df(self.times, df)

        self.mag1 = mag1
        self.mag1.set_df(self.times, df)

        self.baro1 = baro1
        self.baro1.set_df(self.times, df)

        self.gps1 = gps1
        self.gps1.set_df(self.times, df)

    def update(self, input: PhysicsEngineInput) -> bool:
        self.time += self.dt

        return True

    def get_time(self) -> float:
        return self.time

    def get_current_true_data(self) -> PhysicsEngineOutput:
        return None

    def finished(self) -> bool:
        return self.time >= self.times[-1]


class PX4ReplayEnvironment(EnvironmentInterface):
    def __init__(self, ulog_path: str, imu_rate: int = 500, dt: float = 0.001):
        super().__init__()

        topics = load_ulog(ulog_path)

        imu_df = resample_imu(topics['sensor_combined'], imu_rate)
        self.imu1 = ReplayIMUModel(
            rate=imu_rate,
            acc_x='acc_x', acc_y='acc_y', acc_z='acc_z',
            gyro_x='gyro_x', gyro_y='gyro_y', gyro_z='gyro_z',
            acc_range_g=float('inf'),
            gyro_range_deg=float('inf'),
        )
        self.imu1.set_df(
            imu_df['timestamp'].values,
            imu_df,
        )

        mag_df = topics['sensor_mag']
        self.mag1 = EventReplayMagnetometerModel()
        self.mag1.set_data(
            mag_df['timestamp'].values,
            mag_df[['x', 'y', 'z']].values,
        )

        baro_df = topics['sensor_baro']
        self.baro1 = EventReplayBarometerModel()
        self.baro1.set_data(
            baro_df['timestamp'].values,
            baro_df[['pressure', 'temperature', 'height']].values,
        )

        gps_df = topics['vehicle_gps_position']
        self.gps1 = EventReplayGPSModel()
        self.gps1.set_data(
            gps_df['timestamp'].values,
            gps_df[['latitude_deg', 'longitude_deg', 'altitude_msl_m', 'vel_n_m_s', 'vel_e_m_s', 'vel_d_m_s', 'eph', 'epv', 'stddev_speed', 'satellites_used']].values,
        )

        self._ekf2_df = self._build_ekf2_df(
            topics['vehicle_attitude'],
            topics['vehicle_local_position'],
            topics['vehicle_angular_velocity'],
        )

        self.time = 0.0
        self.dt = dt
        self.end_time = imu_df['timestamp'].iloc[-1]
        self.current_state = None

    def update(self, input: PhysicsEngineInput) -> bool:
        self.time += self.dt

        if self.time > self.end_time:
            return False

        self._refresh_ekf2_state()

        return True

    def get_time(self) -> float:
        return self.time

    def get_current_true_data(self) -> PhysicsEngineOutput:
        return self.current_state

    def finished(self) -> bool:
        return self.time >= self.end_time

    def _build_ekf2_df(self, att_df, pos_df, angvel_df) -> pd.DataFrame:
        t = att_df['timestamp'].values
        pt = pos_df['timestamp'].values
        wt = angvel_df['timestamp'].values

        df = pd.DataFrame({'timestamp': t})
        df['qw'] = att_df['q[0]'].values
        df['qx'] = att_df['q[1]'].values
        df['qy'] = att_df['q[2]'].values
        df['qz'] = att_df['q[3]'].values

        for col in ['x', 'y', 'z', 'vx', 'vy', 'vz', 'ax', 'ay', 'az']:
            df[col] = np.interp(t, pt, pos_df[col].values)

        df['x'] -= df['x'].iloc[0]
        df['y'] -= df['y'].iloc[0]
        df['z'] -= df['z'].iloc[0]

        df['wx'] = np.interp(t, wt, angvel_df['xyz[0]'].values)
        df['wy'] = np.interp(t, wt, angvel_df['xyz[1]'].values)
        df['wz'] = np.interp(t, wt, angvel_df['xyz[2]'].values)

        return df

    def _refresh_ekf2_state(self) -> None:
        t = self.time
        times = self._ekf2_df['timestamp'].values

        def interp(col: str) -> float:
            return float(np.interp(t, times, self._ekf2_df[col].values))

        q = np.array([interp('qw'), interp('qx'), interp('qy'), interp('qz')])
        n = np.linalg.norm(q)

        if n > 0:
            q = q / n

        self.current_state = PhysicsEngineOutput(
            acc=np.array([interp('ax'), interp('ay'), interp('az')]),
            vel=np.array([interp('vx'), interp('vy'), interp('vz')]),
            pos=np.array([interp('x'), interp('y'), interp('z')]),
            w=np.array([interp('wx'), interp('wy'), interp('wz')]),
            q=q,
        )
