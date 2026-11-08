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

On the update: `make tidy` may now report functions that are too large. Split them, or suppress
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
