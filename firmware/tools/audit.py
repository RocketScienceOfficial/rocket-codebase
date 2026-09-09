import os
import re
from pathlib import Path
from bus_ownership import check_board


SOURCE_SUFFIXES = {".c", ".cpp", ".h", ".hpp", ".cc", ".hh"}

# Trees that end up on a flight target. platform/host is deliberately absent: it is the SITL
# build, it only ever runs on a development machine, and it legitimately uses std::thread,
# std::map and friends to emulate the RTOS.
SCAN_ROOTS = ("src", "platform")

# Relative to the firmware root. Host-only code that is never cross-compiled.
EXCLUDED_DIRS = (
    "platform/host",
)

# GoogleTest suites are built only with BUILD_TESTS=ON, always against the host platform, so
# they are held to the same standard as platform/host rather than the flight standard.
EXCLUDED_DIR_NAMES = {"tests"}

# A C++ deleted/defaulted member is not an allocation, so `= delete` must not trip the
# `delete` keyword rule below.
DELETED_MEMBER_RE = re.compile(r"=\s*(?:delete|default)\s*;")

BANNED_PATTERNS = [
    (
        "heap allocation",
        re.compile(
            r"\b(?:malloc|calloc|realloc|reallocarray|free|aligned_alloc|posix_memalign"
            r"|alloca|valloc|memalign|strdup|strndup|asprintf|vasprintf|getline)\b"
        ),
    ),
    (
        "operator new/delete",
        re.compile(r"\b(?:new|delete)\b"),
    ),
    (
        "exceptions",
        re.compile(r"\b(?:try|catch|throw|exception)\b"),
    ),
    (
        "STL type",
        re.compile(
            r"\bstd::(?:vector|map|multimap|unordered_map|unordered_set|set|multiset|list"
            r"|forward_list|deque|queue|stack|priority_queue|string|wstring|function"
            r"|shared_ptr|unique_ptr|weak_ptr|make_shared|make_unique|allocator|thread"
            r"|mutex|recursive_mutex|shared_mutex|lock_guard|unique_lock|condition_variable"
            r"|future|promise|async|stringstream|ostringstream|istringstream|regex|any)\b"
        ),
    ),
    (
        "STL type",
        re.compile(r"\b(?:vector|string|array|map|set|deque|list)<"),
    ),
    (
        "banned header",
        re.compile(
            r"#\s*include\s*<(?:vector|map|unordered_map|unordered_set|set|multiset|list"
            r"|forward_list|deque|queue|stack|string|memory|functional|thread|mutex"
            r"|shared_mutex|condition_variable|future|sstream|iostream|ostream|istream"
            r"|fstream|stdexcept|regex|any|new)>"
        ),
    ),
]

# 1. Line comments: //[^\n]*
# 2. Block comments: /* ... */ (using re.DOTALL so .* matches newlines)
# 3. String literals: "..." (to prevent matching keywords inside print statements)
STRIP_PATTERN = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"', re.DOTALL)


def is_in_submodule(file_path, repo_root):
    # A git submodule checkout carries .git as a *file* rather than a directory.
    for parent in file_path.parents:
        if parent == repo_root:
            return False

        if (parent / ".git").is_file():
            return True

    return False


def is_excluded(file_path, repo_root):
    if EXCLUDED_DIR_NAMES.intersection(file_path.parts):
        return True

    relative = file_path.relative_to(repo_root).as_posix()

    return any(relative.startswith(excluded + "/") for excluded in EXCLUDED_DIRS)


def blank_comments_and_strings(content):
    # Replace rather than delete, so line numbers survive for reporting.
    return STRIP_PATTERN.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), content)


def scan_file(file_path):
    content = file_path.read_text(encoding="utf-8", errors="ignore")
    clean_content = blank_comments_and_strings(content)

    findings = []

    for line_number, line in enumerate(clean_content.splitlines(), start=1):
        if DELETED_MEMBER_RE.search(line):
            continue

        for category, pattern in BANNED_PATTERNS:
            for match in pattern.finditer(line):
                findings.append((line_number, category, match.group(0).strip()))

    return findings


def check_unsafe_constructs(repo_root):
    foundIssues = False

    for root in SCAN_ROOTS:
        root_path = repo_root / root

        if not root_path.is_dir():
            print(f"Scan root '{root}' does not exist")
            foundIssues = True
            continue

        for file_path in sorted(root_path.rglob("*")):
            if not file_path.is_file() or file_path.suffix not in SOURCE_SUFFIXES:
                continue

            if is_in_submodule(file_path, repo_root) or is_excluded(file_path, repo_root):
                continue

            for line_number, category, token in scan_file(file_path):
                relative = file_path.relative_to(repo_root).as_posix()
                print(f"{relative}:{line_number}: {category}: '{token}'")
                foundIssues = True

    return foundIssues


def check_bus_ownership(repo_root):
    foundIssues = False

    for board_dir in sorted((repo_root / "boards").iterdir()):
        if not board_dir.is_dir():
            continue

        for message in check_board(board_dir):
            print(message)
            foundIssues = True

    return foundIssues


def main():
    print("Auditing code...")

    repo_root = Path(os.getcwd())

    foundIssues = check_unsafe_constructs(repo_root)
    foundIssues = check_bus_ownership(repo_root) or foundIssues

    if not foundIssues:
        print(f"No issues found (scanned {', '.join(SCAN_ROOTS)}; skipped submodules, {', '.join(EXCLUDED_DIRS)}, and tests/).")
        exit(0)
    else:
        print("Issues found. Please review the above list and address them before proceeding.")
        exit(1)


if __name__ == "__main__":
    main()
