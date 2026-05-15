import numpy as np


np.random.seed(42)  # For reproducibility


_MAX_ALTITUDE = 10000  # Maximum altitude in meters
_MAX_VELOCITY = 500  # Maximum velocity in m/s


# Data generation for training the PINN + Normalization
def generate_collocation_points(num_points):
    h = np.random.uniform(0, _MAX_ALTITUDE, num_points) / _MAX_ALTITUDE
    v = np.random.uniform(0, _MAX_VELOCITY, num_points) / _MAX_VELOCITY

    return np.stack([h, v], axis=1)


# Boundary conditions: at apogee, velocity should be zero + Normalization
def generate_boundary_points(num_points):
    h = np.random.uniform(0, _MAX_ALTITUDE, num_points) / _MAX_ALTITUDE
    v = np.zeros(num_points) / _MAX_VELOCITY
    apogee = h

    return np.stack([h, v, apogee], axis=1)
