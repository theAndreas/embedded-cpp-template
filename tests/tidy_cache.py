#!/usr/bin/env python3
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Checks template/tools/lib/tidy_cache.py: that it skips clang-tidy only where the result
cannot differ.

A project of one source and one header is made in a temporary folder, with the script in its
tools/lib/, and clang-tidy is called through it as run-clang-tidy.py calls it. A clang-tidy of
our own on the PATH notes each call before it hands it to the real one, so that a case sees
whether clang-tidy ran. The cases: a pass is kept and skipped the next time; a finding is never
kept, nor a warning that does not fail; a NOLINT counts, and so does a change of .clang-tidy;
the same project in another folder hits what the first one passed.

Usage:
  tests/tidy_cache.py
"""

import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TEMPLATE = ROOT / "template"
SCRIPT = TEMPLATE / "tools" / "lib" / "tidy_cache.py"
CHECK = "cppcoreguidelines-avoid-non-const-global-variables"
CLEAN = "inline int Value() { return 1; }\n"
FINDING = "int global_value{0};\n"


def real_clang_tidy() -> str:
    """The clang-tidy uv.lock pins, as tools/tidy.sh runs it."""
    result = subprocess.run(
        ["uv", "run", "--frozen", "--project", str(TEMPLATE), "python", "-c",
         "import shutil; print(shutil.which('clang-tidy'))"],
        stdout=subprocess.PIPE,
        text=True,
        check=True,
    )
    return result.stdout.strip()


class Project:
    """A project of src/a.cpp and src/a.h, checked through its copy of the script."""

    def __init__(self, folder: Path, work: Path):
        self.folder = folder
        self.work = work
        (folder / "tools" / "lib").mkdir(parents=True)
        shutil.copy(SCRIPT, folder / "tools" / "lib" / "tidy_cache.py")
        (folder / "src").mkdir()
        (folder / "src" / "a.cpp").write_text('#include "a.h"\n')
        self.configure(f"-*,{CHECK}", "*")
        self.header(CLEAN)
        database = [{
            "directory": str(folder),
            "file": "src/a.cpp",
            "command": "c++ -std=c++11 -MMD -c src/a.cpp -o a.o",
        }]
        (folder / "compile_commands.json").write_text(json.dumps(database))

    def configure(self, checks: str, warnings_as_errors: str) -> None:
        (self.folder / ".clang-tidy").write_text(
            f"Checks: '{checks}'\nWarningsAsErrors: '{warnings_as_errors}'\n")

    def header(self, text: str) -> None:
        (self.folder / "src" / "a.h").write_text(f"#pragma once\n{text}")

    def tidy(self) -> tuple[int, str, bool]:
        """Exit code and output of one call, and whether the real clang-tidy checked the file."""
        log = self.work / "calls"
        log.write_text("")
        source = str(self.folder / "src" / "a.cpp")
        result = subprocess.run(
            [sys.executable, str(self.folder / "tools" / "lib" / "tidy_cache.py"),
             f"-p={self.folder}", "-header-filter=src/", "-quiet", source],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
            env=dict(os.environ, PATH=f"{self.work / 'bin'}{os.pathsep}{os.environ['PATH']}",
                     XDG_CACHE_HOME=str(self.work / "cache")),
        )
        # The script asks clang-tidy for the configuration of the file as well, which is no check.
        checked = any(source in call.split() and "--dump-config" not in call
                      for call in log.read_text().splitlines())
        return result.returncode, result.stdout, checked


def expect(errors: list[str], case: str, outcome: tuple[int, str, bool], passes: bool,
           checked: bool) -> None:
    code, output, ran = outcome
    if (code == 0) != passes or (CHECK in output) == passes or ran != checked:
        errors.append(f"{case}: expected {'a pass' if passes else 'the finding'}, "
                      f"{'checked' if checked else 'skipped'}; got exit {code}, "
                      f"{'checked' if ran else 'skipped'}:\n{output}")


def main() -> int:
    errors: list[str] = []
    with tempfile.TemporaryDirectory() as temporary:
        work = Path(temporary)
        (work / "bin").mkdir()
        shim = work / "bin" / "clang-tidy"
        shim.write_text(f'#!/bin/sh\necho "$@" >> "{work / "calls"}"\n'
                        f'exec "{real_clang_tidy()}" "$@"\n')
        shim.chmod(0o755)

        project = Project(work / "one" / "Demo", work)
        expect(errors, "a clean file", project.tidy(), passes=True, checked=True)
        expect(errors, "the same again", project.tidy(), passes=True, checked=False)

        project.header(FINDING)
        expect(errors, "a finding in the header", project.tidy(), passes=False, checked=True)
        expect(errors, "the finding again", project.tidy(), passes=False, checked=True)

        project.header(FINDING.replace(";", f";  // NOLINT({CHECK})"))
        expect(errors, "the finding under a NOLINT", project.tidy(), passes=True, checked=True)
        expect(errors, "the NOLINT again", project.tidy(), passes=True, checked=False)
        project.header(FINDING)
        expect(errors, "the NOLINT taken out", project.tidy(), passes=False, checked=True)

        project.configure("-*,readability-braces-around-statements", "*")
        expect(errors, "the check off", project.tidy(), passes=True, checked=True)
        project.configure(f"-*,{CHECK}", "*")
        expect(errors, "the check on again", project.tidy(), passes=False, checked=True)

        project.configure(f"-*,{CHECK}", "")
        code, output, ran = project.tidy()
        if code != 0 or CHECK not in output or not ran:
            errors.append(f"a warning that does not fail: got exit {code}:\n{output}")
        code, output, ran = project.tidy()
        if CHECK not in output or not ran:
            errors.append(f"the warning again: kept, not shown:\n{output}")

        project.configure(f"-*,{CHECK}", "*")
        project.header(CLEAN)
        # What the first case passed, so kept since.
        expect(errors, "clean once more", project.tidy(), passes=True, checked=False)
        elsewhere = Project(work / "two" / "Demo", work)
        expect(errors, "the same project in another folder", elsewhere.tidy(), passes=True,
               checked=False)

    for error in errors:
        print(error, file=sys.stderr)
    print(f"tidy_cache.py: {'failed' if errors else 'every case as expected'}")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
