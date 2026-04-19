# THIS SCRIPT IS CALLED BY PLATFORMIO DURING THE BUILD PROCESS TO GENERATE THE C LIBRARY

Import("env")

import subprocess
import os


current_dir = env.subst("$PROJECT_DIR")

datalink_dir = os.path.join(current_dir, "..", "..", "datalink")
generator_script = os.path.join(datalink_dir, "gen.py")
schema_dir = os.path.join(datalink_dir, "schemas")
template_dir = os.path.join(datalink_dir, "templates")

generated_dir = os.path.join(current_dir, "lib", "datalink")
output_header = os.path.join(generated_dir, "datalink.h")
output_source = os.path.join(generated_dir, "datalink.c")


def get_newest_input_time():
    max_mtime = 0

    if os.path.exists(generator_script):
        max_mtime = max(max_mtime, os.path.getmtime(generator_script))

    for directory in [schema_dir, template_dir]:
        for root, _, files in os.walk(directory):
            for file in files:
                filepath = os.path.join(root, file)
                max_mtime = max(max_mtime, os.path.getmtime(filepath))

    return max_mtime


def should_rebuild():
    if not os.path.exists(output_header) or not os.path.exists(output_source):
        return True

    out_mtime = min(os.path.getmtime(output_header), os.path.getmtime(output_source))

    return get_newest_input_time() > out_mtime


if should_rebuild():
    print("Generating C library...")

    result = subprocess.run([
        "python",
        generator_script,
        "--lang", "c",
        "--outdir", generated_dir,
    ])
else:
    print("C library is up to date, skipping generation.")
