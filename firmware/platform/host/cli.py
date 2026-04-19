import subprocess
from pathlib import Path

ROOT = Path(__file__).parent
BUILD = ROOT / "build"


def run_cmd(cmd, cwd=None):
    try:
        subprocess.check_call(cmd, cwd=cwd)
    except:
        pass


def build(mode=None, log_level=None, freerun=None):
    BUILD.mkdir(exist_ok=True)

    run_cmd(["cmake",
             "-S", ".",
             "-B", "build",
             "-G", "Ninja",
             ("-DCMAKE_BUILD_TYPE=" + mode) if mode else None,
             ("-DOBC_LOG_LEVEL=" + log_level) if log_level else None,
             "-DOBC_SITL_FREERUN=" + ("ON" if freerun == True else "OFF"),
             "-DOBC_TESTS=OFF",
             ], cwd=ROOT)

    run_cmd(["ninja"], cwd=BUILD)


def test():
    if not BUILD.exists():
        print("Build the project before running tests.")
        return

    run_cmd(["cmake",
             "-S", ".",
             "-B", "build",
             "-G", "Ninja",
             "-DOBC_TESTS=ON",
             ], cwd=ROOT)

    run_cmd(["ninja"], cwd=BUILD)
    run_cmd(["ctest", "--output-on-failure"], cwd=BUILD)


def clean():
    import shutil

    if BUILD.exists():
        shutil.rmtree(BUILD)


def run():
    exe = BUILD / "firmware"

    run_cmd([str(exe)], cwd=BUILD)
