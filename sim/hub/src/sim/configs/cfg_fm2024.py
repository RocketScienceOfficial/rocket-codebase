from sim.env.environments import *


def get_environment(dt: float):
    return SequentialReplayEnvironment(
        resample_dt=dt,
        file_path="./data/FM2024.csv",
        time_col="time",
        imu1=ReplayIMUData(dt=0.002, acc_x="acc1_x", acc_y="acc1_y", acc_z="acc1_z", gyro_x="gyro1_x", gyro_y="gyro1_y", gyro_z="gyro1_z"),
        mag1=ReplayMagnetometerData(dt=0.01, mag_x="mag_x", mag_y="mag_y", mag_z="mag_z"),
        baro1=ReplayBarometerData(dt=0.004, pressure="pressure", temperature="temperature", height="baro_height"),
        gps1=ReplayGPSData(dt=0.04, lat="latitude", lon="longitude", alt="altitude", sats="sats"),
    )
