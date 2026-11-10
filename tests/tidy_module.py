#!/usr/bin/env python3
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Checks the template's clang-tidy module, tools/lib/tidy_module/, against tests/tidy_module/.

Each file there marks the lines a check reports with a comment `house: <how>`; how is `fix`
for a report with a fix-it, or a word that the message contains for one without. Every other
line must not be reported. Then the fix-its are applied to a copy, which has to compile as
C++11 and, checked again, to keep only the reports without a fix-it.

Usage:
  tests/tidy_module.py

The module is built by template/tools/lib/tidy_module.sh into build/tidy-module, against the
headers that script finds: in the dev container, or in LLVM_INCLUDE_DIR.
"""

import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEMPLATE = ROOT / "template"
CASES = ROOT / "tests" / "tidy_module"
STANDARD = "-std=c++11"
MARK = re.compile(r"//\s*house:\s*(\w+)")
REPORT = re.compile(
    r"^(?P<file>[^:\s]+):(?P<line>\d+):\d+: warning: (?P<message>.*) \[house-[\w-]+\]$"
)


def build_module() -> Path:
    """Builds the module; the script says on stderr why, if it cannot."""
    result = subprocess.run(
        [
            str(TEMPLATE / "tools" / "lib" / "tidy_module.sh"),
            str(ROOT / "build" / "tidy-module"),
        ],
        stdout=subprocess.PIPE,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        sys.exit(1)
    return Path(result.stdout.strip().splitlines()[-1])


def clang_tidy(module: Path, source: Path, *extra: str) -> dict[int, str]:
    """Runs the module's checks on source; returns the message of each line reported."""
    result = subprocess.run(
        [
            "uv",
            "run",
            "--frozen",
            "--project",
            str(TEMPLATE),
            "clang-tidy",
            f"--load={module}",
            "--config={Checks: '-*,house-*'}",
            "--quiet",
            *extra,
            str(source),
            "--",
            STANDARD,
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    reports = {}
    for line in result.stdout.splitlines():
        match = REPORT.match(line)
        if match and Path(match["file"]).resolve() == source.resolve():
            reports[int(match["line"])] = match["message"]
    if result.returncode != 0 and not reports:
        sys.exit(f"clang-tidy failed on {source}:\n{result.stdout}")
    return reports


def marks(source: Path) -> dict[int, str]:
    found = {}
    for number, line in enumerate(source.read_text().splitlines(), start=1):
        match = MARK.search(line)
        if match:
            found[number] = match[1]
    return found


def compare(
    source: Path, expected: dict[int, str], reports: dict[int, str]
) -> list[str]:
    errors = []
    for line in sorted(expected.keys() | reports.keys()):
        how = expected.get(line)
        message = reports.get(line)
        where = f"{source.name}:{line}"
        if how is None:
            errors.append(f"{where}: reported, but not marked: {message}")
        elif message is None:
            errors.append(f"{where}: marked `house: {how}`, but not reported")
        elif how == "fix" and ";" in message:
            errors.append(
                f"{where}: marked for a fix-it, reported without one: {message}"
            )
        elif how != "fix" and how not in message:
            errors.append(f"{where}: marked `house: {how}`, reported as: {message}")
    return errors


def check_fixed(
    module: Path, source: Path, expected: dict[int, str], work: Path
) -> list[str]:
    """Applies the fix-its to a copy; it must compile and keep only the reports without one."""
    copy = work / source.name
    shutil.copyfile(source, copy)
    clang_tidy(module, copy, "--fix")
    errors = []
    compiler = os.environ.get("CXX", "c++")
    compiled = subprocess.run(
        [compiler, STANDARD, "-fsyntax-only", str(copy)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    if compiled.returncode != 0:
        errors.append(
            f"{source.name}, with the fix-its applied, does not compile:\n{compiled.stdout}"
        )
    left = {line: how for line, how in expected.items() if how != "fix"}
    errors += [
        f"after the fix-its: {error}"
        for error in compare(source, left, clang_tidy(module, copy))
    ]
    return errors


def main() -> int:
    module = build_module()
    errors = []
    with tempfile.TemporaryDirectory() as work:
        for source in sorted(CASES.glob("*.cpp")):
            expected = marks(source)
            errors += compare(source, expected, clang_tidy(module, source))
            errors += check_fixed(module, source, expected, Path(work))
    for error in errors:
        print(error, file=sys.stderr)
    if errors:
        return 1
    print(
        f"tidy_module: all cases pass ({', '.join(p.name for p in sorted(CASES.glob('*.cpp')))})"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
