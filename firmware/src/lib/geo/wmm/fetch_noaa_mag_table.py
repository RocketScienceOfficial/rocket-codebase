"""
Resources:
    - https://github.com/PX4/PX4-ECL/blob/master/geo_lookup/fetch_noaa_table.py
    - https://github.com/PX4/PX4-Autopilot/blob/main/src/lib/world_magnetic_model/fetch_noaa_table.py
"""

import math
import json
import os
import urllib.request


KEY = "gFE5W"
BASE_URL = "https://www.ngdc.noaa.gov/geomag-web/calculators/calculateIgrfgrid"

SAMPLING_RES = 10
SAMPLING_MIN_LAT = -90
SAMPLING_MAX_LAT = 90
SAMPLING_MIN_LON = -180
SAMPLING_MAX_LON = 180

LAT_DIM = int((SAMPLING_MAX_LAT - SAMPLING_MIN_LAT) / SAMPLING_RES) + 1
LON_DIM = int((SAMPLING_MAX_LON - SAMPLING_MIN_LON) / SAMPLING_RES) + 1


def _get_data(component, key):
    table = []
    index = 0
    metadata = None

    for latitude in range(SAMPLING_MIN_LAT, SAMPLING_MAX_LAT + 1, SAMPLING_RES):
        params = urllib.parse.urlencode(
            {
                "key": KEY,
                "lat1": latitude,
                "lat2": latitude,
                "lon1": SAMPLING_MIN_LON,
                "lon2": SAMPLING_MAX_LON,
                "latStepSize": 1,
                "lonStepSize": SAMPLING_RES,
                "magneticComponent": component,
                "resultFormat": "json",
            }
        )

        f = urllib.request.urlopen(BASE_URL + "?%s" % params)
        data = json.loads(f.read())

        if metadata is None:
            metadata = {
                "model": data["model"],
                "version": data["version"],
                "date": data["result"][0]["date"],
            }

        table.append([])

        for p in data["result"]:
            table[index].append(p[key])

        index += 1

    return table, metadata


def _get_declination():
    return _get_data("d", "declination")


def _get_inclination():
    return _get_data("i", "inclination")


def _get_strength():
    return _get_data("f", "totalintensity")


def _generate_table_code(table, metadata, name, unit):
    code = ""
    code += f"// Magnetic {name} in {unit}\n"
    code += f"// Model: {metadata['model']}\n"
    code += f"// Version: {metadata['version']}\n"
    code += f"// Date: {metadata['date']}\n"
    code += f"static const int16_t {str(name).upper()}_TABLE[LAT_DIM][LON_DIM] = {{\n"

    for i in range(len(table)):
        code += "\t{"

        for j in range(len(table[i])):
            v = int(round(math.radians(table[i][j]) * 10000)) if "radians" in unit else int(round(table[i][j] / 10))

            code += str(v) + ", "

        code += "},\n"

    code += "};\n"

    return code


def generate_code():
    path = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(path, "mag_tables.h")

    with open(file_path, "w") as file:
        file.write(f"#ifndef _MAG_TABLES_H\n#define _MAG_TABLES_H\n\n")
        file.write(f"#include <stdint.h>\n\n")
        file.write(f"#define SAMPLING_RES {SAMPLING_RES}\n")
        file.write(f"#define SAMPLING_MIN_LAT {SAMPLING_MIN_LAT}\n")
        file.write(f"#define SAMPLING_MAX_LAT {SAMPLING_MAX_LAT}\n")
        file.write(f"#define SAMPLING_MIN_LON {SAMPLING_MIN_LON}\n")
        file.write(f"#define SAMPLING_MAX_LON {SAMPLING_MAX_LON}\n")
        file.write(f"#define LAT_DIM {LAT_DIM}\n")
        file.write(f"#define LON_DIM {LON_DIM}\n")
        file.write("\n")

        declination_table, declination_metadata = _get_declination()
        inclination_table, inclination_metadata = _get_inclination()
        strength_table, strength_metadata = _get_strength()

        file.write(f"{_generate_table_code(declination_table, declination_metadata, 'declination', 'radians * 10^4')}\n")
        file.write(f"{_generate_table_code(inclination_table, inclination_metadata, 'inclination', 'radians * 10^4')}\n")
        file.write(f"{_generate_table_code(strength_table, strength_metadata, 'strength', 'Gauss * 10^4')}\n")

        file.write(f"#endif")


if __name__ == "__main__":
    generate_code()
