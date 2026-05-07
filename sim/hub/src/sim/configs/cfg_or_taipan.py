from sim.env.environments import *
from sim.env.physics_engines import *


def get_environment(dt: float):
    return SyntheticEnvironment(
        engine=SimpleIntegratorPhysicsEngine(dt=dt, scenario=OpenRocketSimScenario(file_path="./data/OR_TAIPAN.csv", preferred_time=30.0)),
        imu1=SyntheticIMUModel(rate=500, noise_acc=GaussianNoiseModel(mean=0, std_dev=0.055), noise_gyro=GaussianNoiseModel(mean=0, std_dev=0.17), acc_range_g=6.0, gyro_range=500.0),
        mag1=SyntheticMagnetometerModel(rate=100, noise=GaussianNoiseModel(mean=0, std_dev=0.0004)),
        baro1=SyntheticBarometerModel(rate=50, noise=GaussianNoiseModel(mean=0, std_dev=0.4)),
        gps1=SyntheticGPSModel(rate=25, noise_hor=GaussianNoiseModel(mean=0, std_dev=1.7), noise_ver=GaussianNoiseModel(mean=0, std_dev=3.1), noise_vel=GaussianNoiseModel(mean=0, std_dev=0.5), lat=50.337497, lon=19.525838, alt=30),
    )
