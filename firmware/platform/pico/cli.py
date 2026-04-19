import subprocess
import shutil
from pathlib import Path

ROOT = Path(__file__).parent
BUILD = ROOT / "build"


def _run_cmd(cmd, cwd=None):
    try:
        subprocess.check_call(cmd, cwd=cwd)

        return True
    except:
        return False


def build(mode=None, log_level=None):
    BUILD.mkdir(exist_ok=True)

    _run_cmd(["cmake",
             "-S", ".",
              "-B", "build",
              "-G", "Ninja",
              ("-DCMAKE_BUILD_TYPE=" + mode) if mode else None,
              ("-DOBC_LOG_LEVEL=" + log_level) if log_level else None,
              ], cwd=ROOT)
    _run_cmd(["ninja"], cwd=BUILD)


def clean():
    if BUILD.exists():
        shutil.rmtree(BUILD)


def run():
    print("Doesn't supported")