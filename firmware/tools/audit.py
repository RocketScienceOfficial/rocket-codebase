import os
import re
from pathlib import Path


def main():
    print("Auditing platforms...")

    repo_root = Path(os.getcwd())
    p = repo_root / "src"
    foundIssues = False

    keyword_pattern = re.compile(r'\b(malloc|free|new|delete|try|catch|exception|vector<|string<|array<)\b')

    # 1. Line comments: //[^\n]*
    # 2. Block comments: /* ... */ (using re.DOTALL so .* matches newlines)
    # 3. String literals: "..." (to prevent matching keywords inside print statements)
    strip_pattern = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"', re.DOTALL)

    for file_path in p.rglob("*"):
        if any((parent / ".git").is_file() for parent in file_path.parents if repo_root in parent.parents or parent == repo_root):
            continue

        if file_path.is_file() and file_path.suffix in ['.c', '.cpp', '.h', '.hpp', '.cc', '.hh']:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
                clean_content = strip_pattern.sub('', content)
                matches = keyword_pattern.findall(clean_content)

                if matches:
                    print(f"Found keywords in {file_path}: {', '.join(set(matches))}")
                    foundIssues = True

    if not foundIssues:
        print("No issues found.")
        exit(0)
    else:
        print("Issues found. Please review the above list and address them before proceeding.")
        exit(1)


if __name__ == "__main__":
    main()
