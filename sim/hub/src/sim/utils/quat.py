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
    """Rotate 3-vector v by unit quaternion q = [w,x,y,z] (scalar-first, Hamilton)."""
    w, x, y, z = q
    vx, vy, vz = v
    tx = 2.0 * (y * vz - z * vy)
    ty = 2.0 * (z * vx - x * vz)
    tz = 2.0 * (x * vy - y * vx)
    ox = vx + w * tx + (y * tz - z * ty)
    oy = vy + w * ty + (z * tx - x * tz)
    oz = vz + w * tz + (x * ty - y * tx)
    return np.array([ox, oy, oz])


def ned_to_body(q_ned_from_body: np.ndarray, v_ned: np.ndarray) -> np.ndarray:
    """Express NED vector in body frame. q rotates body -> NED."""
    return quat_rotate_vector(quat_conj(q_ned_from_body), v_ned)
