#!/usr/bin/env python3
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Checks template/tools/avr_stack.py on calls through a pointer, against the cases below.

Each case is a program of one file, built for an ATmega328P by avr-gcc from apt, as
cmake/toolchains/avr-gcc.cmake builds one: with the call graphs and, unless the case says not,
with its relocations kept. A call through a pointer is taken to reach every function whose address
the program takes, the deepest of them counted; a file of rules says which of those a caller
never reaches through a pointer, each with its reason. A case passes where the script ends as
the case wants and prints what it wants.

Usage:
  tests/avr_stack.py
"""

import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SCRIPT = ROOT / "template" / "tools" / "avr_stack.py"
MCU = "atmega328p"

# main() calls one of two operations through a table; Deep() has the larger frame, so the
# deepest path goes through it. volatile, so that avr-gcc cannot see which one is called and
# turn the call into a direct one.
TABLE = """
namespace demo {
using Operation = unsigned char (*)(unsigned char);

unsigned char Shallow(unsigned char value) { return value + 1; }

__attribute__((noinline)) unsigned char Deep(unsigned char value) {
  volatile unsigned char buffer[64];
  for (unsigned char index = 0; index < sizeof(buffer); ++index) {
    buffer[index] = value;
  }
  return buffer[value & 63];
}

Operation volatile table[] = {Shallow, Deep};
}  // namespace demo

int main() {
  volatile unsigned char choice = 1;
  return demo::table[choice](choice);
}
"""

# Apply() calls the operation it is given through a pointer, and its own address is taken as
# well, so a call through a pointer may reach Apply() as far as the image tells: a recursion,
# which Apply() never makes, as a rule says.
TRAMPOLINE = """
namespace demo {
using Operation = unsigned char (*)(unsigned char);
using Applier = unsigned char (*)(Operation, unsigned char);

__attribute__((noinline)) unsigned char Twice(unsigned char value) { return value * 2; }

__attribute__((noinline)) unsigned char Apply(Operation operation, unsigned char value) {
  return operation(value);
}

Operation volatile operation = Twice;
Applier volatile applier = Apply;
}  // namespace demo

int main() { return demo::applier(demo::operation, 3); }
"""

APPLY_NEVER_APPLY = """
[[never]]
caller = "demo::Apply(*"
callee = "demo::Apply(*"
reason = "Apply() calls the operation it is given, which is never Apply() itself."
"""

NO_REASON = """
[[never]]
caller = "demo::Apply(*"
callee = "demo::Apply(*"
"""


def build(source: str, work: Path, relocations: bool) -> Path:
    """The program of `source`, linked in `work`; its call graph lies beside it."""
    work.mkdir()
    (work / "main.cpp").write_text(source)
    flags = [f"-mmcu={MCU}", "-Os", "-fstack-usage", "-fcallgraph-info=su"]
    subprocess.run(["avr-g++", *flags, "-c", "main.cpp", "-o", "main.o"], cwd=work, check=True)
    link = [f"-mmcu={MCU}", "-Wl,--gc-sections"]
    if relocations:
        link.append("-Wl,--emit-relocs")
    subprocess.run(["avr-g++", *link, "main.o", "-o", "main.elf"], cwd=work, check=True)
    return work / "main.elf"


def run(elf: Path, rules: str | None = None) -> tuple[int, str]:
    """The script on `elf`, with the rules given in a file of their own; its status and output."""
    command = [sys.executable, str(SCRIPT), "--mcu", MCU, "--paths"]
    if rules is not None:
        path = elf.with_name("stack.toml")
        path.write_text(rules)
        command += ["--never", str(path)]
    command += [str(elf), str(elf.parent)]
    done = subprocess.run(command, capture_output=True, text=True, check=False)
    return done.returncode, done.stdout + done.stderr


def expect(name: str, result: tuple[int, str], status: int, *wanted: str) -> list[str]:
    """What is wrong with `result`: another status than `status`, or a text of `wanted` missing."""
    code, output = result
    errors = []
    if code != status:
        errors.append(f"{name}: ended with {code}, not {status}")
    errors += [f"{name}: does not say {text!r}" for text in wanted if text not in output]
    if errors:
        errors.append(f"{name}: it said:\n{output}")
    return errors


def main() -> int:
    errors = []
    with tempfile.TemporaryDirectory() as directory:
        work = Path(directory)
        table = build(TABLE, work / "table", relocations=True)
        errors += expect("a table of operations", run(table), 0, "demo::Deep(unsigned char)")
        bare = build(TABLE, work / "bare", relocations=False)
        errors += expect("a program without its relocations", run(bare), 1, "--emit-relocs")
        trampoline = build(TRAMPOLINE, work / "trampoline", relocations=True)
        errors += expect("a trampoline without a rule", run(trampoline), 1, "recursive",
                         "demo::Apply(unsigned char (*)(unsigned char), unsigned char)")
        errors += expect("a trampoline with its rule", run(trampoline, APPLY_NEVER_APPLY), 0,
                         "demo::Twice(unsigned char)")
        errors += expect("a rule without a reason", run(trampoline, NO_REASON), 1, "reason")
    for error in errors:
        print(error, file=sys.stderr)
    if not errors:
        print("avr_stack.py: every case as expected.")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
