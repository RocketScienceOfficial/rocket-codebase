import argparse
import re
import subprocess
import sys
from pathlib import Path

from ram_report import find_tool

# Exact symbol names that mean an allocator, the C++ free store, or the exception runtime made
# it into the image. The _r suffixed forms are newlib's reentrant entry points, which is what
# the ARM builds actually call into.
BANNED_SYMBOLS = {
    "malloc", "calloc", "realloc", "reallocarray", "free",
    "_malloc_r", "_calloc_r", "_realloc_r", "_free_r",
    "aligned_alloc", "memalign", "_memalign_r", "posix_memalign", "valloc", "pvalloc",
    "strdup", "strndup", "_strdup_r", "_strndup_r",
    "sbrk", "_sbrk", "_sbrk_r",
    "__cxa_throw", "__cxa_allocate_exception", "__cxa_begin_catch", "__cxa_end_catch",
    "__cxa_rethrow", "__gxx_personality_v0", "_Unwind_RaiseException", "_Unwind_Resume",
    # Mangled operator new/delete, in case the symbol comes through undemangled.
    "_Znwj", "_Znwm", "_Znaj", "_Znam", "_ZdlPv", "_ZdaPv", "_ZdlPvj", "_ZdlPvm",
}

# Demangled operator new/delete carry argument lists, so they are matched by prefix.
BANNED_PREFIXES = ("operator new", "operator delete")

OBJECT_SUFFIXES = {".o", ".obj", ".a"}

# `nm` prints "path:" header lines when given an archive or several files.
HEADER_RE = re.compile(r"^(\S.*):$")
SYMBOL_RE = re.compile(r"^\s*([0-9a-fA-F]*)\s*([A-Za-z?])\s+(.+?)\s*$")


def is_banned(name, allowed):
    if name in allowed:
        return False

    if name in BANNED_SYMBOLS:
        return True

    return name.startswith(BANNED_PREFIXES)


def run_nm(nm_bin, extra_args, paths):
    result = subprocess.run([nm_bin, "-C", *extra_args, *[str(p) for p in paths]], capture_output=True, text=True)

    if result.returncode != 0 and not result.stdout:
        print(result.stderr.strip(), file=sys.stderr)
        sys.exit(1)

    return result.stdout


def parse_nm(output, default_owner):
    owner = default_owner

    for line in output.splitlines():
        if not line.strip():
            continue

        header = HEADER_RE.match(line)
        if header:
            owner = header.group(1)
            continue

        match = SYMBOL_RE.match(line)
        if match:
            _, sym_type, name = match.groups()
            yield owner, sym_type, name


def collect_objects(root):
    if root.is_file():
        return [root]

    return sorted(p for p in root.rglob("*") if p.is_file() and p.suffix in OBJECT_SUFFIXES)


def audit_image(nm_bin, elf_path, allowed):
    # Anything the linker actually pulled in and defined. If the allocator is here, some
    # translation unit asked for it.
    output = run_nm(nm_bin, ["--defined-only"], [elf_path])

    found = sorted({name for _, _, name in parse_nm(output, str(elf_path)) if is_banned(name, allowed)})

    print(f"== Banned symbols defined in {elf_path} ==")

    if not found:
        print("  none")
    else:
        for name in found:
            print(f"  {name}")

    return found


def audit_objects(nm_bin, root, allowed):
    # Undefined references name the object that requested the symbol, which is what you
    # actually need in order to remove it.
    objects = collect_objects(root)

    if not objects:
        print(f"No object files or archives found under {root}")
        return {}

    callers = {}

    # Batched so a large build directory does not blow the command line length limit.
    for i in range(0, len(objects), 64):
        batch = objects[i:i + 64]
        output = run_nm(nm_bin, ["--undefined-only"], batch)

        for owner, _, name in parse_nm(output, str(batch[0])):
            if is_banned(name, allowed):
                callers.setdefault(name, set()).add(owner)

    print(f"\n== Banned symbols referenced by objects under {root} ({len(objects)} scanned) ==")

    if not callers:
        print("  none")

    for name in sorted(callers):
        print(f"  {name}")
        for owner in sorted(callers[name]):
            print(f"      <- {owner}")

    return callers


def main():
    parser = argparse.ArgumentParser(description="Audit a built firmware image for heap, free-store and exception-runtime symbols. Complements tools/audit.py, which only greps source.")
    parser.add_argument("target", help="Path to a firmware .elf, or a build directory to walk for .o/.a files")
    parser.add_argument("--toolchain-bin", help="Directory containing arm-none-eabi-nm (skips auto-detect)")
    parser.add_argument("--nm", help="nm binary to use (overrides --toolchain-bin and auto-detect)")
    parser.add_argument("--allow", action="append", default=[], metavar="SYM", help="Symbol to treat as acceptable; repeatable")
    parser.add_argument("--report-only", action="store_true", help="Always exit 0, just print the report")
    args = parser.parse_args()

    target = Path(args.target)

    if not target.exists():
        print(f"No such file or directory: {target}", file=sys.stderr)
        sys.exit(1)

    if args.nm:
        nm_bin = args.nm
    elif args.toolchain_bin:
        nm_bin = str(Path(args.toolchain_bin) / "arm-none-eabi-nm")
    else:
        nm_bin = find_tool("arm-none-eabi-nm")

    allowed = set(args.allow)

    found = False

    if target.is_file() and target.suffix != ".a":
        found = bool(audit_image(nm_bin, target, allowed))

        build_dir = target.parent
        found = bool(audit_objects(nm_bin, build_dir, allowed)) or found
    else:
        found = bool(audit_objects(nm_bin, target, allowed))

    if found and not args.report_only:
        print("\nBanned symbols present. Note that newlib can pull in malloc via printf-family formatting; use --allow to accept a symbol once you have confirmed why it is there.")
        sys.exit(1)

    print("\nNo banned symbols." if not found else "\nReported only; not failing.")
    sys.exit(0)


if __name__ == "__main__":
    main()
