"""
Resources:
    - https://en.wikipedia.org/wiki/Geographic_coordinate_conversion
    - https://en.wikipedia.org/wiki/Local_tangent_plane_coordinates
    - https://en.wikipedia.org/wiki/World_Geodetic_System#WGS_84    
    - https://www.movable-type.co.uk/scripts/latlong.html
"""

from math import pi, sin, cos, sqrt, atan, atan2, radians, degrees
import numpy as np


a = 6378137.0
b = 6356752.3142
r = 6371000.0
e = 0.081819190842622
e2 = 0.00669437999014
f = 1 / 298.257223563

g = 9.80665
g_ned = np.array([0, 0, g], dtype=float)
mag_ned = np.array([0.191, 0.019, 0.462], dtype=float)


# ================== UTILS ==================


def _convert_coords_to_rad(coords_list):
    new_coords = []

    for x in coords_list:
        new_coords.append(radians(x))

    return new_coords


def _convert_coords_to_deg(coords_list):
    new_coords = []

    for x in coords_list:
        new_coords.append(degrees(x))

    return new_coords


# ================== GEO ==================


# REF: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_geodetic_to_ECEF_coordinates
# REF: https://www.mathworks.com/help/map/ref/geodetic2ecef.html
def geo_to_ecef(lat, lon, alt):
    n = a / sqrt(1 - e2 * sin(lat) ** 2)

    x = (n + alt) * cos(lat) * cos(lon)
    y = (n + alt) * cos(lat) * sin(lon)
    z = ((1 - e2) * n + alt) * sin(lat)

    return [x, y, z]


# REF: https://www.mathworks.com/help/aeroblks/ecefpositiontolla.html
# REF: https://www.mathworks.com/help/map/ref/geodetic2ecef.html
# REF: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_geodetic_coordinates
def ecef_to_geo(x, y, z):
    lon = atan2(y, x)

    s = sqrt(x**2 + y**2)

    beta = atan(z / ((1 - f) * s))
    miu = atan((z + e2 * (1 - f) / (1 - e2) * a * sin(beta) ** 3) / (s - e2 * a * cos(beta) ** 3))
    err = 1e10

    while err > 1e-10:
        beta = atan((1 - f) * sin(miu) / cos(miu))
        last_miu = miu
        miu = atan((z + e2 * (1 - f) / (1 - e2) * a * sin(beta) ** 3) / (s - e2 * a * cos(beta) ** 3))
        err = last_miu - miu

    n = a / sqrt(1 - e2 * sin(miu) ** 2)
    h = s * cos(miu) + (z + e2 * n * sin(miu)) * sin(miu) - n

    return [miu, lon, h]


# REF: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ECEF_to_ENU
# REF: https://en.wikipedia.org/wiki/Local_tangent_plane_coordinates
# REF: https://www.mathworks.com/help/map/ref/ecef2ned.html
def ecef_to_ned(x, y, z, lat0, lon0, alt0):
    [x0, y0, z0] = geo_to_ecef(lat0, lon0, alt0)

    dx = x - x0
    dy = y - y0
    dz = z - z0

    x_ned = dx * (-sin(lat0) * cos(lon0)) + dy * (-sin(lat0) * sin(lon0)) + dz * (cos(lat0))
    y_ned = dx * (-sin(lon0)) + dy * (cos(lon0)) + dz * (0)
    z_ned = dx * (-cos(lat0) * cos(lon0)) + dy * (-cos(lat0) * sin(lon0)) + dz * (-sin(lat0))

    return [x_ned, y_ned, z_ned]


# REF: https://en.wikipedia.org/wiki/Geographic_coordinate_conversion#From_ENU_to_ECEF
# REF: https://en.wikipedia.org/wiki/Local_tangent_plane_coordinates
# REF: https://www.mathworks.com/help/map/ref/ned2ecef.html
def ned_to_ecef(x, y, z, lat0, lon0, alt0):
    [x0, y0, z0] = geo_to_ecef(lat0, lon0, alt0)

    x_ecef = x * (-sin(lat0) * cos(lon0)) + y * (-sin(lon0)) + z * (-cos(lat0) * cos(lon0))
    y_ecef = x * (-sin(lat0) * sin(lon0)) + y * (cos(lon0)) + z * (-cos(lat0) * sin(lon0))
    z_ecef = x * (cos(lat0)) + y * (0) + z * (-sin(lat0))

    x_ecef += x0
    y_ecef += y0
    z_ecef += z0

    return [x_ecef, y_ecef, z_ecef]


# REF: https://www.mathworks.com/help/map/ref/geodetic2ned.html
def geo_to_ned(lat0, lon0, alt0, lat1, lon1, alt1):
    [lat0, lon0, lat1, lon1] = _convert_coords_to_rad([lat0, lon0, lat1, lon1])

    [x1, y1, z1] = geo_to_ecef(lat1, lon1, alt1)
    [x, y, z] = ecef_to_ned(x1, y1, z1, lat0, lon0, alt0)

    return [x, y, z]


# REF: https://www.mathworks.com/help/map/ref/ned2geodetic.html
def ned_to_geo(lat0, lon0, alt0, x, y, z):
    [lat0, lon0] = _convert_coords_to_rad([lat0, lon0])

    [x_ecef, y_ecef, z_ecef] = ned_to_ecef(x, y, z, lat0, lon0, alt0)
    [lat, lon, alt] = ecef_to_geo(x_ecef, y_ecef, z_ecef)
    [lat, lon] = _convert_coords_to_deg([lat, lon])

    return [lat, lon, alt]


# ================== OTHER ==================


# REF: https://github.com/PX4/PX4-ECL/blob/master/geo/geo.cpp
# REF: https://www.movable-type.co.uk/scripts/latlong.html
def geo_distance(lat0, lon0, lat1, lon1):
    [lat0, lon0, lat1, lon1] = _convert_coords_to_rad([lat0, lon0, lat1, lon1])

    d_lat = lat1 - lat0
    d_lon = lon1 - lon0

    a = sin(d_lat / 2) * sin(d_lat / 2) + sin(d_lon / 2) * sin(d_lon / 2) * cos(lat0) * cos(lat1)
    c = 2 * atan2(sqrt(a), sqrt(1 - a))

    return r * c


# REF: https://github.com/PX4/PX4-ECL/blob/master/geo/geo.cpp
# REF: https://www.movable-type.co.uk/scripts/latlong.html
def geo_bearing(lat0, lon0, lat1, lon1):
    [lat0, lon0, lat1, lon1] = _convert_coords_to_rad([lat0, lon0, lat1, lon1])

    cos_lat1 = cos(lat1)
    d_lon = lon1 - lon0

    y = sin(d_lon) * cos_lat1
    x = cos(lat0) * sin(lat1) - sin(lat0) * cos_lat1 * cos(d_lon)
    theta = atan2(y, x)
    bearing = (theta + 2 * pi) % (2 * pi)

    return bearing


def baro_formula(press):
    return 44330.76923 * (1 - ((press / 101325) ** 0.1902632))


# ================== TEST CASES ==================
# REF: Matlab (Mathworks)


if __name__ == "__main__":
    print(geo_to_ecef(radians(48.8562), radians(2.3508), radians(0.0674)))
    print(_convert_coords_to_deg(ecef_to_geo(4200952.481741992, 172458.5026025355, 4780052.075474019)))
    print(ecef_to_ned(1345660, -4350891, 4452314, radians(44.532), radians(-72.782), 1699))
    print(ned_to_ecef(1334.3044602, -2544.36768413, 359.96087162, radians(44.532), radians(-72.782), 1699))
    print(geo_to_ned(44.532, -72.782, 1699, 44.544, -72.814, 1340))
    print(ned_to_geo(44.532, -72.782, 1699, 1334.3044602, -2544.36768413, 359.96087162))
    print(geo_distance(44.532, -72.782, 44.544, -72.814))
    print(degrees(geo_bearing(44.532, -72.782, 44.544, -72.814)))
