import numpy as np


def quat_conj(q: np.ndarray) -> np.ndarray:
    return np.array([q[0], -q[1], -q[2], -q[3]])


def quat_multiply(q: np.ndarray, p: np.ndarray) -> np.ndarray:
    w1, x1, y1, z1 = q
    w2, x2, y2, z2 = p

    return np.array(
        [
            w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2,
            w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
            w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2,
            w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2,
        ]
    )


def quat_rotate_vector(q: np.ndarray, v: np.ndarray) -> np.ndarray:
    return quat_multiply(quat_multiply(q, np.array([0.0, v[0], v[1], v[2]])), quat_conj(q))[1:]
