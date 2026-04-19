import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent


def run_step(name: str, cmd: list[str]) -> None:
    print(f"\n==> {name}")
    print("$", " ".join(cmd))

    result = subprocess.run(cmd, cwd=ROOT)
    if result.returncode != 0:
        raise RuntimeError(f"Step failed: {name}")


def main() -> int:
    steps = [
        ("Generate Python output", [sys.executable, "gen.py", "--lang", "python", "--outdir", "tests/python"]),
        ("Run Python tests", [sys.executable, "-m", "pytest", "tests/python", "-q"]),
        ("Generate C# output", [sys.executable, "gen.py", "--lang", "csharp", "--outdir", "tests/csharp"]),
        ("Run C# tests", ["dotnet", "run", "--project", "tests/csharp/TestRuntime.csproj"]),
        ("Generate C output", [sys.executable, "gen.py", "--lang", "c", "--outdir", "tests/c"]),
        ("Build C tests", ["gcc", "-std=c11", "tests/c/datalink.c", "tests/c/test_runtime.c", "-o", "tests/c/test_runtime"]),
        ("Run C tests", ["tests/c/test_runtime"]),
    ]

    try:
        for name, cmd in steps:
            run_step(name, cmd)
    except RuntimeError as exc:
        print(f"\n==> FAILED: {exc}")
        return 1

    print("\n==> All language tests passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
