#!/usr/bin/env python3
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Checks template/tools/lib/doc_indent.py against the cases in tests/doc_indent/.

Each case <name>.h is what clang-format may leave; <name>.expected.h is what --fix makes of
it, and a case without one has to stay as it is. For each, --check fails exactly where --fix
changes something, and a file --fix has made passes --check and stays as it is under a second
--fix: tools/lint.sh runs --fix after clang-format on every source, and one that went on
changing would never pass. And the width is the ColumnLimit of the project's .clang-format: a
block wrapped at 100 columns passes in a project that sets 100 and is rewrapped in one that sets
none.

Usage:
  tests/doc_indent.py
"""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "template" / "tools" / "lib" / "doc_indent.py"
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


# A brief wrapped at 100 columns, as a project with ColumnLimit: 100 has it.
WIDE = """/*!
 * \\brief The letter of the port that carries TCA0's waveform outputs: B, unless the build defines
 *        another.
 */
"""


def check_column_limit(work: Path) -> list[str]:
    """The block passes where .clang-format sets 100 columns, and not where it sets none."""
    errors = []
    for limit, wanted in (("ColumnLimit: 100\n", 0), ("", 1)):
        project = work / f"limit{wanted}"
        (project / "tools" / "lib").mkdir(parents=True)
        shutil.copyfile(SCRIPT, project / "tools" / "lib" / "doc_indent.py")
        (project / ".clang-format").write_text("BasedOnStyle: Google\n" + limit)
        source = project / "wide.h"
        source.write_text(WIDE)
        status = subprocess.run(
            [sys.executable, str(project / "tools" / "lib" / "doc_indent.py"), "--check",
             str(source)],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=False).returncode
        if status != wanted:
            errors.append(f"a block at 100 columns, {limit.strip() or 'no ColumnLimit'}: "
                          f"--check ended with {status}, not {wanted}")
    return errors


def main() -> int:
    cases = sorted(p for p in CASES.glob("*.h") if not p.name.endswith(".expected.h"))
    errors = []
    with tempfile.TemporaryDirectory() as work:
        for case in cases:
            errors += check_case(case, Path(work))
        errors += check_column_limit(Path(work))
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
