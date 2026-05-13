import numpy as np
import pandas as pd
from pyulog import ULog
from sim.utils.geo import baro_formula


_REQUIRED_TOPICS = [
    'sensor_combined',
    'sensor_mag',
    'sensor_baro',
    'vehicle_gps_position',
    'vehicle_attitude',
    'vehicle_local_position',
    'vehicle_angular_velocity',
]


def load_ulog(path: str) -> dict[str, pd.DataFrame]:
    ulog = ULog(path)

    raw = {}
    for d in ulog.data_list:
        if d.multi_id == 0 and d.name in _REQUIRED_TOPICS:
            raw[d.name] = d.data

    missing = [t for t in _REQUIRED_TOPICS if t not in raw]
    if missing:
        raise ValueError(f"ulog missing required topics: {missing}")

    t0 = min(raw[t]['timestamp'][0] for t in _REQUIRED_TOPICS)

    result = {}
    for topic in _REQUIRED_TOPICS:
        df = pd.DataFrame(raw[topic])
        df['timestamp'] = (df['timestamp'] - t0) / 1e6
        result[topic] = df

    _convert_gps(result['vehicle_gps_position'])
    _add_baro_height(result['sensor_baro'])

    return result


def _convert_gps(df: pd.DataFrame) -> None:
    df['stddev_speed'] = np.sqrt(df['s_variance_m_s'].clip(0))
    df.drop(columns=['s_variance_m_s'], inplace=True)


def _add_baro_height(df: pd.DataFrame) -> None:
    df['height'] = df['pressure'].apply(baro_formula)


def resample_imu(sc_df: pd.DataFrame, rate_hz: int) -> pd.DataFrame:
    t_start = sc_df['timestamp'].iloc[0]
    t_end = sc_df['timestamp'].iloc[-1]
    t = np.arange(t_start, t_end, 1.0 / rate_hz)
    ts = sc_df['timestamp'].values

    return pd.DataFrame({
        'timestamp': t,
        'acc_x': np.interp(t, ts, sc_df['accelerometer_m_s2[0]'].values),
        'acc_y': np.interp(t, ts, sc_df['accelerometer_m_s2[1]'].values),
        'acc_z': np.interp(t, ts, sc_df['accelerometer_m_s2[2]'].values),
        'gyro_x': np.interp(t, ts, sc_df['gyro_rad[0]'].values),
        'gyro_y': np.interp(t, ts, sc_df['gyro_rad[1]'].values),
        'gyro_z': np.interp(t, ts, sc_df['gyro_rad[2]'].values),
    })
