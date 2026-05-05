from sim.env.environments import *


def get_environment(dt: float):
    return SequentialReplayEnvironment(
        resample_dt=dt,
        file_path="./data/FM2024.csv",
        time_col="time",
        imu1=ReplayIMUModel(rate=500, acc_x="acc1_x", acc_y="acc1_y", acc_z="acc1_z", gyro_x="gyro1_x", gyro_y="gyro1_y", gyro_z="gyro1_z", clipping_acc_range=6.0, clipping_gyro_range=500.0),
        mag1=ReplayMagnetometerModel(rate=100, mag_x="mag_x", mag_y="mag_y", mag_z="mag_z"),
        baro1=ReplayBarometerModel(rate=50, pressure="pressure", temperature="temperature", height="baro_height"),
        gps1=ReplayGPSModel(rate=25, lat="latitude", lon="longitude", alt="altitude", vel_n=None, vel_e=None, vel_d=None, sats="sats", stddev_hor=None, stddev_ver=None, stddev_vel=None),
    )
