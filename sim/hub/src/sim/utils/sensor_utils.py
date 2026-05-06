from sim import datalink
import numpy as np


def get_imu_clipping_flags(acc_meas, gyro_meas, acc_clipping_range, gyro_clipping_range):
    flags = 0

    if abs(acc_meas[0]) > acc_clipping_range:
        flags |= datalink.imu_clipping_flags.DATALINK_FLAGS_IMU_CLIP_ACC_X
    if abs(acc_meas[1]) > acc_clipping_range:
        flags |= datalink.imu_clipping_flags.DATALINK_FLAGS_IMU_CLIP_ACC_Y
    if abs(acc_meas[2]) > acc_clipping_range:
        flags |= datalink.imu_clipping_flags.DATALINK_FLAGS_IMU_CLIP_ACC_Z
    if abs(gyro_meas[0]) > gyro_clipping_range:
        flags |= datalink.imu_clipping_flags.DATALINK_FLAGS_IMU_CLIP_GYRO_X
    if abs(gyro_meas[1]) > gyro_clipping_range:
        flags |= datalink.imu_clipping_flags.DATALINK_FLAGS_IMU_CLIP_GYRO_Y
    if abs(gyro_meas[2]) > gyro_clipping_range:
        flags |= datalink.imu_clipping_flags.DATALINK_FLAGS_IMU_CLIP_GYRO_Z

    return flags


def get_values_interp(time, times, vals):
    new_data = []

    for col in range(vals.shape[1]):
        new_data.append(np.interp(time, times, vals[:, col]))

    return new_data


class SensorDelayedBuffer:
    def __init__(self, delay_ms, initial_value):
        self.delay_ms = delay_ms
        self.initial_value = initial_value
        self.buffer = []

    def update(self, current_time_sec, new_value):
        self.buffer.append((current_time_sec + self.delay_ms / 1000 - 1e-5, new_value))

        last_val = self.initial_value

        while self.buffer and self.buffer[0][0] <= current_time_sec:
            last_val = self.buffer.pop(0)[1]

        return last_val
