#!/usr/bin/env python3
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Checks template/tools/doc_indent.py against the cases in tests/doc_indent/.

Each case <name>.h is what clang-format may leave; <name>.expected.h is what --fix makes of
it, and a case without one has to stay as it is. For each, --check fails exactly where --fix
changes something, and a file --fix has made passes --check and stays as it is under a second
--fix: tools/lint.sh runs --fix after clang-format on every source, and one that went on
changing would never pass.

Usage:
  tests/doc_indent.py
"""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "template" / "tools" / "doc_indent.py"
CASES = ROOT / "tests" / "doc_indent"


def run(mode: str, file: Path, stderr: int | None = None) -> int:
    """Runs the script on one file; returns its exit status."""
    return subprocess.run(
        [sys.executable, str(SCRIPT), mode, str(file)],
        stdout=subprocess.DEVNULL,
        stderr=stderr,
        check=False,
    ).returncode


def check_case(case: Path, work: Path) -> list[str]:
    expected_file = case.with_name(f"{case.stem}.expected.h")
    if not expected_file.is_file():
        expected_file = case
    expected = expected_file.read_bytes()
    copy = work / case.name
    shutil.copyfile(case, copy)

    errors = []
    status = run("--check", copy)
    wanted = 0 if expected_file == case else 1
    if status != wanted:
        errors.append(f"{case.name}: --check ended with {status}, not {wanted}")
    run("--fix", copy)
    if copy.read_bytes() != expected:
        errors.append(f"{case.name}: --fix does not make {expected_file.name} of it")
        return errors
    status = run("--check", copy)
    if status != 0:
        errors.append(f"{case.name}: --check fails on what --fix made of it")
    run("--fix", copy)
    if copy.read_bytes() != expected:
        errors.append(f"{case.name}: a second --fix changes what the first made")
    return errors


def main() -> int:
    cases = sorted(p for p in CASES.glob("*.h") if not p.name.endswith(".expected.h"))
    errors = []
    with tempfile.TemporaryDirectory() as work:
        for case in cases:
            errors += check_case(case, Path(work))
        if run("--neither", Path(work) / cases[0].name, subprocess.DEVNULL) != 2:
            errors.append("a call without --fix or --check does not end with 2")
    for error in errors:
        print(error, file=sys.stderr)
    if errors:
        return 1
    print(f"doc_indent: all cases pass ({', '.join(case.name for case in cases)})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
