# Changes

What changed in each version of the template, and what a project has to do when `copier update`
brings it.

## Unreleased

- **Checks of our own, as a clang-tidy module:** `tools/tidy_module/`, the module `house`, which
  `tools/tidy_module.sh` builds and `tools/tidy.sh` loads; `.clang-tidy` turns on `house-*`. The
  first check, `house-braced-member-init`, holds a constructor's member initializer list to braced
  initialization, with a fix-it where braces keep the meaning: reported without one where an
  argument narrows or the type is a template's parameter, and not at all where braces would choose
  a `std::initializer_list` constructor. The template's own cases are in `tests/tidy_module/`.
- **LLVM's headers in the dev container and in CI**, which the module is compiled against:
  `.devcontainer/llvm-headers.sh` from apt.llvm.org, at the major version `ARG LLVM_MAJOR` of the
  Dockerfile names; the action `llvm-headers` in `static-analysis.yml`.
- **cppcheck reads `__has_include` in C++11:** `tools/cppcheck.sh` passes
  `-D__has_include(x)=1`, as cppcheck knows it from C++17 only and stopped at it with a syntax
  error, where GCC and Clang accept it in every standard.
- **git signs in to GitHub through gh**: the dev container's `postAttachCommand` runs
  `gh auth setup-git` on every attach, so that `git push` takes gh's token, whose login stays in
  its volume, instead of VS Code's helper, which opens a window on the host. Without a login it
  does nothing.

On the update: rebuild the dev container, for the headers. A member initializer written in
parentheses now fails `make tidy`; `tools/tidy.sh` applies nothing, but clang-tidy's `--fix`
swaps the parentheses where it says so.

## Unreleased

- **`docs/CodingStyle.md`: how the code is written**, each rule with an id and what checks it:
  files, functions, types, classes, memory on the microcontroller, errors, names, suppressions.
  The C++ Core Guidelines, MISRA C++:2023 and the Google style, in that order; where it departs
  from Google, and why. Template parameters are named for their role, not `T`.
- **`docs/Testing.md`: how the code is tested**: which of the three kinds of test proves what,
  how a test is named and written, what the mock of the core may do, and what SelfCheck checks.
- **Rules for every target, and a profile per target**: CodingStyle.md holds for any target from
  an 8-bit microcontroller to the host; `docs/targets/avr.md` holds what the AVR alone needs, as
  text in flash, its critical sections and its registers.
- **An expression does one thing** (CodingStyle.md T-18): a computation or condition of several
  steps is split into named steps.
- **More rules in CodingStyle.md**: the promotion of a narrow type to `int`, widened before the
  arithmetic (T-19); arrays that know their size (T-20); a `char` widened through `unsigned char`
  (T-21); swappable parameters as types of their own (FN-16); a member that must exist as a
  `reference_wrapper` by M-3 (FN-15); a `switch` over an `enum class` that handles an unnamed
  value after it (T-15); objects of static storage made when compiling (M-5); constants in a
  header whose address is taken (M-4); pointer casts across targets (T-9); no downcast (M-2); no
  `virtual` for a test (C-6); no cycle of includes (F-14); examples fit to ship (F-15); reserved
  names; and how a suppression is reasoned (S-1, S-4, S-5).
- **`Discard()`** in `attributes.h`, for a result dropped on purpose (CodingStyle.md FN-10): before
  C++17 GCC warns through a cast to `void`, clang does not, so the cast passed on the host and
  failed for the target.
- **More rules in Testing.md**: the tests follow the documentation, and an exclusion from coverage
  says why, which `tools/coverage.sh` checks (TS-4); every test passes under the sanitizers
  (TS-24); arguments outside the contract (TS-25); calls in the wrong order and every transition
  of a state machine (TS-26); a table of what differs between host and target.
- **Logical constness** (CodingStyle.md FN-11): a function is `const` when it leaves unchanged
  what a user sees, the pin, register or buffer it controls through a pointer included; one that
  writes a register through a member pointer is not `const`, though it compiles as one.
- **Reuse before writing** (CodingStyle.md M-3): what the project has, then the standard library
  where every target has it, then a library like it such as the ETL or Utl; only what none has
  is written, once and for reuse.
- **Files grouped in folders**, a folder per module with header and source side by side, and
  `test/` mirroring `src/`; a test of `RingBuffer.h` is `RingBufferTest.cpp`, which clang-format
  and cpplint now take for the header's test. CMake finds sources and tests in every folder.
- **`docs/Misra.md`: MISRA C++:2023 rule by rule**, what covers each and where the project
  deviates; a draft, to be checked against the standard.
- **The size of a function is limited**: 40 lines of body, 4 parameters, 3 levels of nesting, a
  cognitive complexity of 15; a test has no limit of lines.
- **`{namespace}/attributes.h` in the skeleton**: `PREFIX_NODISCARD`, `[[nodiscard]]` from C++17
  and GCC's `warn_unused_result` before it; the skeleton's functions use it. A project made
  before takes the file from a new copy.
- **Integers of fixed width in `src/`**, a loop counter among them (CodingStyle.md T-1).
- **`AGENTS.md` for a coding agent**, which any agent reads, and `CLAUDE.md`, which imports it for
  Claude Code: what to read before writing code, and what to run after.

On the update: `make tidy` may now report functions that are too large.
`CMakeLists.txt` is the project's and is not updated: to build a module in a folder of its own,
take its two `file(GLOB_RECURSE ...)` lines and the `list(FILTER ...)` from a new copy. Split them, or suppress
one with its reason. Notes of the project's own for an agent go at the end of `AGENTS.md`.

## v0.2.1

- `tools/coverage.sh` fails when the tests left no counts, rather than reporting 0 of 0 lines.

On the update from v0.1.0: remove `build/` once. The presets name the compiler now, and CMake
makes a build directory configured with another one anew, dropping the preset's options with
the old cache: `--coverage` and warnings as errors went missing without a word.

## v0.2.0

The workflow as Wordclock grew it: one way in, warnings as errors in CI only, one pinned
environment for the Python tools.

- **A Makefile names every build and check.** `make` lists them, `make check` is what CI runs,
  `make lint` the checks of seconds. Every step of every workflow is one of its targets. A
  project's own targets go into `tools/project.mk`; one added to `PROJECT_CHECKS` is part of
  `make check`.
- **Warnings are errors in CI and under `make check`, not in a developer's build:** the `ci-`
  presets turn on `CMAKE_COMPILE_WARNING_AS_ERROR`, chosen by `STRICT`, which `CI=true` sets.
  `cmake/checks.cmake` no longer adds `-Werror`.
- **The tests a second time by clang:** the preset and target `clang`, and a CI job.
- **Every header of `src/` compiled on its own**, for the host and for the board:
  `make headers`, `tools/self_contained_headers.py`.
- **The Python tools pinned in `pyproject.toml` and `uv.lock`**, run with `uv run --frozen`:
  clang-format, clang-tidy, cpplint, gcovr, shellcheck, actionlint, ruff and copier. The
  template's Dependabot keeps them and the actions current. ruff reads the Python scripts.
- **CI:** `static-analysis.yml` beside `ci.yml`, each with its badge; `release.yml` publishes a
  tag whose checks pass and whose version is that of `library.properties`, with the library as a
  `.zip`; `permissions: contents: read`; composite actions `apt-install` (cached, a stalled
  mirror retried), `uv-sync` and `arduino-setup` (arduino-cli at the Dockerfile's version, the
  core cached).
- `make arduino-lint` checks what git has, not the working tree with its `.venv`.
- The dev container installs clang, and syncs the Python tools when it is created.

On the update: move a project's own steps of `ci.yml` into a workflow of its own, and its own
commands into `tools/project.mk`. `.venv/` is ignored now; a `uvx` call of a project's own
script becomes `uv run --frozen`.

## v0.1.0

The template, out of Utl's checks, CI and dev container, made general:

- What is the project's lies in `tools/project.sh`: the board, the core, the variants, and the
  hooks `variant_flags()`, `prepare()` and `check_elf()`. `tools/setup.sh` installs the core and
  runs `prepare()`, in the dev container and in CI.
- clang-tidy adds the C++ Core Guidelines and CERT, without the aliases of checks that are on
  and without what the AVR cannot have; each exception says why.
- cppcheck beside clang-tidy, from MaxMatrix: `tools/cppcheck.sh`. A project whose code
  cppcheck cannot parse sets `CPPCHECK=0`, with the reason.
- The host build warns with `-Wconversion -Wsign-conversion -Wshadow` as well, from
  `cmake/checks.cmake`.
- `.editorconfig`, from MaxMatrix.
- `cppcoreguidelines-special-member-functions` accepts a class that deletes its copy without
  declaring its moves; `test/.clang-tidy` lets a test cast as `F()` does and keep globals.
- The starting point of a new library, `src/`, `test/` and the examples, comes only with
  `copier copy` and the answer `skeleton`; a project brought in answers no.
