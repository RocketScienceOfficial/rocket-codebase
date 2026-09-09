import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

RAM_SECTIONS = {
    ".ram_vector_table",
    ".data",
    ".bss",
    ".uninitialized_data",
    ".heap",
    ".stack_dummy",
    ".stack1_dummy",
    ".scratch_x",
    ".scratch_y",
    ".tdata",
    ".tbss",
}

# nm symbol types worth reporting: BSS, data, and weak objects (C++17
# `inline static` members, e.g. pubsub topic storage, land here instead of B/D).
SYMBOL_TYPES = set("bBdDvV")


def find_tool(name):
    exe = shutil.which(name) or shutil.which(name + ".exe")
    if exe:
        return exe

    patterns = [
        f"C:/Program Files*/Arm GNU Toolchain arm-none-eabi/*/bin/{name}.exe",
        f"D:/Programs/Arm GNU Toolchain arm-none-eabi/*/bin/{name}.exe",
    ]

    for pattern in patterns:
        matches = sorted(glob.glob(pattern))
        if matches:
            return matches[-1]

    print(f"Could not find '{name}'. Pass --toolchain-bin <dir> or add it to PATH.", file=sys.stderr)
    sys.exit(1)


def run_size(size_bin, elf_path):
    out = subprocess.run([size_bin, "-A", "-d", elf_path], capture_output=True, text=True, check=True).stdout

    sections = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[0] in RAM_SECTIONS:
            sections[parts[0]] = int(parts[1])

    return sections


def run_nm(nm_bin, elf_path, top):
    out = subprocess.run([nm_bin, "--print-size", "--size-sort", "-C", elf_path], capture_output=True, text=True, check=True).stdout

    symbols = []
    for line in out.splitlines():
        match = re.match(r"^\S+\s+([0-9a-fA-F]+)\s+(\S)\s+(.+)$", line)
        if not match:
            continue

        size_hex, sym_type, name = match.groups()
        if sym_type not in SYMBOL_TYPES or "guard variable" in name:
            continue

        symbols.append((int(size_hex, 16), sym_type, name))

    symbols.sort(reverse=True)
    return symbols[:top]


def main():
    parser = argparse.ArgumentParser(description="Report static RAM usage from a built firmware .elf")
    parser.add_argument("elf", help="Path to firmware.elf")
    parser.add_argument("--top", type=int, default=40, help="Number of largest symbols to list (default 40)")
    parser.add_argument("--toolchain-bin", help="Directory containing arm-none-eabi-size/nm (skips auto-detect)")
    args = parser.parse_args()

    if not Path(args.elf).is_file():
        print(f"No such file: {args.elf}", file=sys.stderr)
        sys.exit(1)

    if args.toolchain_bin:
        size_bin = os.path.join(args.toolchain_bin, "arm-none-eabi-size.exe")
        nm_bin = os.path.join(args.toolchain_bin, "arm-none-eabi-nm.exe")
    else:
        size_bin = find_tool("arm-none-eabi-size")
        nm_bin = find_tool("arm-none-eabi-nm")

    sections = run_size(size_bin, args.elf)

    print("== RAM-resident sections ==")
    total = 0
    for name, size in sections.items():
        total += size
        print(f"{name:<22} {size:>8} B  ({size / 1024:.2f} KiB)")

    print(f"{'TOTAL':<22} {total:>8} B  ({total / 1024:.2f} KiB)")

    print(f"\n== Top {args.top} static RAM consumers (.bss/.data + weak/inline-static) ==")
    for size, sym_type, name in run_nm(nm_bin, args.elf, args.top):
        print(f"{size:>7} B  [{sym_type}]  {name}")


if __name__ == "__main__":
    main()
