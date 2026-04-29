from sim.env.environments import *
from sim.env.physics_engines import *
from sim.env.sensors import *


def get_environment(dt: float):
    return SyntheticEnvironment(
        engine=SimulinkPhysicsEngine(dt=dt, model="acs"),
        imu1=IMUModel(rate=500, noise=GaussianNoiseModel(mean=0, var=0.01)),
        mag1=MagnetometerModel(rate=100, noise=GaussianNoiseModel(mean=0, var=0.1)),
        baro1=BarometerModel(rate=250, noise=GaussianNoiseModel(mean=0, var=0.5)),
        gps1=GPSModel(rate=25, noise=GaussianNoiseModel(mean=0, var=5), lat=50.337497, lon=19.525838, alt=30),
    )
