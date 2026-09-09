from dataclasses import dataclass
from .sensors import *


@dataclass
class Site:
    lat: float
    lon: float
    alt: float


@dataclass
class Board:
    imu: str
    mag: str
    baro: str
    gps: str


@dataclass
class SensorSet:
    imu1: SyntheticIMUModel
    mag1: SyntheticMagnetometerModel
    baro1: SyntheticBarometerModel
    gps1: SyntheticGPSModel


DEFAULT_SITE = Site(lat=50.337497, lon=19.525838, alt=30)


SENSOR_FACTORIES = {
    "bmi088": lambda: SyntheticIMUModel(rate=500, noise_acc=GaussianNoiseModel(mean=0, std_dev=0.22), noise_gyro=GaussianNoiseModel(mean=0, std_dev=0.17), acc_range_g=12.0, gyro_range_deg=500.0),
    "imu_degraded": lambda: SyntheticIMUModel(rate=500, noise_acc=GaussianNoiseModel(mean=0, std_dev=0.3), noise_gyro=GaussianNoiseModel(mean=0, std_dev=0.17), acc_range_g=6.0, gyro_range_deg=500.0),
    "mmc5983ma": lambda: SyntheticMagnetometerModel(rate=100, noise=GaussianNoiseModel(mean=0, std_dev=0.0004)),
    "ms5611": lambda: SyntheticBarometerModel(rate=50, noise=GaussianNoiseModel(mean=0, std_dev=0.4)),
    "baro_degraded": lambda: SyntheticBarometerModel(rate=25, noise=GaussianNoiseModel(mean=0, std_dev=3.0)),
    "ubx_neo_m9n": lambda lat=DEFAULT_SITE.lat, lon=DEFAULT_SITE.lon, alt=DEFAULT_SITE.alt: SyntheticGPSModel(rate=25, noise_hor=GaussianNoiseModel(mean=0, std_dev=1.7), noise_ver=GaussianNoiseModel(mean=0, std_dev=3.1), noise_vel=GaussianNoiseModel(mean=0, std_dev=0.5), lat=lat, lon=lon, alt=alt),
}


BOARDS = {
    "obc": Board(imu="bmi088", mag="mmc5983ma", baro="ms5611", gps="ubx_neo_m9n"),
    "degraded": Board(imu="imu_degraded", mag="mmc5983ma", baro="baro_degraded", gps="ubx_neo_m9n"),
}


def build_sensors(board_name: str, *, gps_site: tuple[float, float, float] | None = None) -> SensorSet:
    if board_name not in BOARDS:
        raise ValueError(f"Unknown board '{board_name}'. Available boards: {', '.join(sorted(BOARDS))}")

    board = BOARDS[board_name]
    gps_kwargs = {} if gps_site is None else dict(zip(("lat", "lon", "alt"), gps_site))

    return SensorSet(
        imu1=SENSOR_FACTORIES[board.imu](),
        mag1=SENSOR_FACTORIES[board.mag](),
        baro1=SENSOR_FACTORIES[board.baro](),
        gps1=SENSOR_FACTORIES[board.gps](**gps_kwargs),
    )
