#!/usr/bin/env python3

import argparse
import importlib.util
import sys
import re
from pathlib import Path

ROOT = Path(__file__).parent.resolve()
PLATFORM_DIR = ROOT / "platform"
LOG_LEVELS = {"debug": "4", "info": "3", "warn": "2", "error": "1", "off": "0"}


def call_platform_function(platform, function, *args):
    module_path = PLATFORM_DIR / platform / "cli.py"

    if not module_path.exists():
        print(f"Platform '{platform}' has no cli.py")
        sys.exit(1)

    spec = importlib.util.spec_from_file_location(platform, module_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    if not hasattr(module, function):
        print(f"Platform '{platform}' does not support '{function}'")
        return

    getattr(module, function)(*args)


def list_platforms_cmd():
    print("Available platforms:")

    for p in PLATFORM_DIR.iterdir():
        if (p / "cli.py").exists():
            print(f"  - {p}")


def audit_cmd():
    print("Auditing platforms...")

    p = Path(ROOT / "src")
    foundIssues = False

    keyword_pattern = re.compile(r'\b(malloc|free|new|delete|try|catch|exception|vector<|string<|array<)\b')

    # 1. Line comments: //[^\n]*
    # 2. Block comments: /* ... */ (using re.DOTALL so .* matches newlines)
    # 3. String literals: "..." (to prevent matching keywords inside print statements)
    strip_pattern = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"', re.DOTALL)

    for file_path in p.rglob("*"):
        if file_path.is_file() and file_path.suffix in ['.c', '.cpp', '.h']:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
                clean_content = strip_pattern.sub('', content)
                matches = keyword_pattern.findall(clean_content)

                if matches:
                    print(f"Found keywords in {file_path}: {', '.join(set(matches))}")
                    foundIssues = True

    if not foundIssues:
        print("No issues found.")
    else:
        print("Issues found. Please review the above list and address them before proceeding.")
        exit(1)


def test_cmd():
    call_platform_function("host", "test")
    return


def main():
    parser = argparse.ArgumentParser(prog="obc")
    sub = parser.add_subparsers(dest="command")

    sub.add_parser("list")
    sub.add_parser("audit")
    sub.add_parser("test")

    build_cmd = sub.add_parser("build")
    build_cmd.add_argument("platform")
    build_cmd.add_argument("--release", action="store_true")
    build_cmd.add_argument("--debug", action="store_true")
    build_cmd.add_argument("--freerun", action="store_true")
    build_cmd.add_argument("--log-level", type=str, choices=list(LOG_LEVELS.keys()))

    run_cmd = sub.add_parser("run")
    run_cmd.add_argument("platform")

    clean_cmd = sub.add_parser("clean")
    clean_cmd.add_argument("platform")

    args = parser.parse_args()

    if args.command == "list":
        list_platforms_cmd()
        return
    elif args.command == "audit":
        audit_cmd()
        return
    elif args.command == "test":
        test_cmd()
        return

    if not args.platform:
        parser.print_help()
        return

    if args.command == "build":
        mode = "Debug" if args.debug else ("Release" if args.release else None)
        log_level = LOG_LEVELS.get(args.log_level) if args.log_level else None
        freerun = args.freerun if args.platform == "host" else None
        call_platform_function(args.platform, "build", mode, log_level, freerun)
    elif args.command == "clean":
        call_platform_function(args.platform, "clean")
    elif args.command == "run":
        call_platform_function(args.platform, "run")
    else:
        print(f"Unknown command '{args.command}'")
        parser.print_help()


if __name__ == "__main__":
    main()
