# Roadmap

Where the template and the projects made from it are going: what is done, what comes next, and
the ideas not yet decided. The open items carry an id, so that a commit, a pull request or a
discussion can name one. A done item moves to its milestone's list; `CHANGELOG.md` says what
each version of the template brings.

## Milestones

### M1 - The template, v0.1.0 (done)

Utl's checks, CI and dev container, made general, with copier to make and update a project.
See `CHANGELOG.md` for what it brings beyond Utl: the C++ Core Guidelines and CERT in
clang-tidy, cppcheck, `-Wconversion`, the warnings an IDE such as CLion reports, and
`tools/project.sh` for what is the project's.

### M2 - Utl on the template (next)

| Id | Item | Status |
|---|---|---|
| T-1 | Bring Utl in with `copier copy --overwrite`; its variants `ram`, `flash`, `checked`, the plugin `avr-flash-vtbl` and the Keypad workflow into `tools/project.sh` and hooks | open |
| T-2 | Fix what the new warnings find in Utl: `-Wconversion`, `-Wsign-conversion`, `-Wshadow`, about ten places | open |
| T-3 | Answer the new Core Guidelines findings in Utl: `reinterpret_cast`, union access, special members, macros; each fixed or NOLINT with its reason | open |
| T-4 | `CPPCHECK=0` in Utl until cppcheck parses `src/utl/array.h:181` (2.19 cannot); report it upstream | open |
| T-5 | `tools/docs.sh` failed silently on a log without a warning: fixed in the template, comes with T-1 | open |

### M2b - Wordclock's workflow, v0.2.0

What Wordclock (AndreasBur/Wordclock) grew on 2026-11-06 and 07, into the template, and from
there into Utl with `copier update`: the first update a project takes, and so the proof that the
mechanism carries.

| Id | Item | Status |
|---|---|---|
| W-1 | One way in: a Makefile that names every build and check, `make` lists them, `make check` is what CI runs and `make lint` the checks of seconds; every step of every workflow is one of its targets | open |
| W-2 | Warnings as errors in CI and under `make check`, not in a developer's build: `ci-` presets with `CMAKE_COMPILE_WARNING_AS_ERROR`, chosen by `STRICT`, which `CI=true` sets | open |
| W-3 | One pinned Python environment, `pyproject.toml` and `uv.lock`, for clang-format, cpplint, clang-tidy, gcovr, shellcheck, actionlint and ruff, the same in the image and in CI, instead of a pin per `uvx` call | open |
| W-4 | Composite actions: `apt-install`, the downloads cached and a stalled mirror retried; `uv-sync`, uv at the Dockerfile's version | open |
| W-5 | Dependabot for the actions and `uv.lock`, weekly, a week's cooldown | open |
| W-6 | `permissions: contents: read`; `ci.yml` callable (`workflow_call`), and `release.yml` on a tag: CI passes, the tag matches `version=` of `library.properties`, the release is published | open |
| W-7 | `static-analysis.yml` of its own, beside `ci.yml`, for a badge of its own: clang-tidy, cppcheck, the Doxygen blocks | open |
| W-8 | The host tests a second time with clang, as a preset | open |
| W-9 | Every header compiled as the only include of an empty file (`self-contained-headers.py`), per target | open |
| W-10 | The sizes the README states checked against the build (`documented-sizes.py`); Utl's table of flash and RAM | open |
| W-11 | The AVR's deepest stack against the RAM it has (`avr-stack.py`), for an Arduino sketch | idea |
| W-12 | ruff over the Python scripts, with the checks that answer "is this wrong" | open |
| W-13 | A CLAUDE.md of the conventions every project shares, from Wordclock's and Utl's | idea |

### M3 - The other projects

| Id | Item | Status |
|---|---|---|
| R-1 | MaxMatrix: bring in; take into the template what it does better (`cross-clang-tidy.py`, the apt cache of `apt-install.sh`, the Wokwi simulation) | open |
| R-2 | XmegaTimer: bring in; ATxmega E5 with a third-party core (`ARDUINO_BOARD_URLS`), no simavr (`SIMAVR_MCU=""`) | open |
| R-3 | TimerTwo: bring in, or retire it if Utl or the ETL covers it | open |
| R-4 | Wordclock: find where it lies, then as R-1 | open |

### M4 - Checks of our own

| Id | Item | Status |
|---|---|---|
| C-1 | Find out whether the clang-tidy pinned from PyPI loads a plugin (`-load`); if not, clang-tidy and the LLVM headers from apt, pinned in the Dockerfile | open |
| C-2 | A clang-tidy module of our own, with the first check: braced initialization in a member initializer list, with a fix-it (Utl's `function_ref.h:106` slipped through) | open |
| C-3 | Further candidates: a NOLINT names its check and its reason; the rules of `cpp-braced-initialization` that no check covers | idea |
| C-4 | Rule of six: a class that declares one special member declares all six, the default constructor too; `cppcoreguidelines-special-member-functions` checks the five, nothing the sixth | idea |
| C-5 | Fixed-width integer types (MISRA C:2012 Dir 4.6, AUTOSAR A3-9-1): no `int`, `unsigned` in `src/`, but `char` for text, `bool`, `size_t`, and what the Arduino core's signatures give, as the `int` of `analogRead()`. `google-runtime-int`, which is on, reports only `short` and `long`. Utl keeps to it already | idea |

### M5 - A generic embedded library on the ETL

| Id | Item | Status |
|---|---|---|
| E-1 | A second flavor of the template, chosen by a copier question: CMake first, the library for any target, Arduino as one user of it | open |
| E-2 | A CI matrix of C++11, 14, 17 and 20, GCC and Clang, on the host, plus avr-gcc through arduino-cli; the ETL's own CI is the model: GCC and Clang from C++11 to C++26, cross builds, a build without the STL | open |
| E-3 | The [ETL](https://github.com/ETLCPP/etl) (MIT) as a pinned dependency: CMake for the host, `etl-arduino` for Arduino; it ships CMake first, and Meson, Bazel (since 2026-04), PlatformIO and Arduino besides. A library of our own on top for what it lacks or does otherwise: errors as `Result` with groups, text and tables in flash. Recommended over a fork, which would have to follow a very active upstream; a fix goes upstream | idea |
| E-4 | Decide what of Utl moves onto the ETL: it has `variant`, `optional`, `expected`, `span`, `string_view`, `vector`, `delegate`, `inplace_function`, `queue_spsc_isr` | idea |

## Ideas, not yet decided

| Id | Idea |
|---|---|
| I-1 | `utl::variant`, without exceptions, `visit` without a table of function pointers in RAM; or `etl::variant` after E-4 |
| I-2 | The host warnings of `cmake/checks.cmake` for GCC as well: `-Wold-style-cast`, `-Wzero-as-null-pointer-constant`, `-Wundef`, `-Wnull-dereference`, which only clang-tidy passes now |
| I-3 | A flavor for host programs, such as FontCreator |
| I-4 | MISRA C++:2023 and AUTOSAR C++14: evaluate PC-lint Plus first, which Vector's own projects use, then PVS-Studio and Cppcheck Premium; only if a project needs the evidence |
| I-5 | Utl's names: the API mimics the standard in part and Google in part; decide one scheme for a library that mimics the standard |
| I-6 | A test of what must not compile in the skeleton, as an example of `add_compile_fail_tests()` |
| I-7 | clangd from PyPI is one major version behind clang-tidy (22 against 23); follow when 23 is there |

## Decisions

- **Initialization in the member initializer list**, C.49, is checked by
  `cppcoreguidelines-prefer-member-initializer`; a default the same for every constructor goes
  into the class, C.48, by `modernize-use-default-member-init`. Both are on.

- **copier, not a GitHub template**: a GitHub template is copied once and then drifts; copier
  updates a project as a three-way merge.
- **The template's files under GPL-3.0-or-later**, as Utl's were; a project picks its own licence.
- **cert-\* without its aliases**: on Utl it reported only what bugprone- checks report already,
  each wanting a second NOLINT.
- **The bounds profile of the Core Guidelines off**: it wants `gsl::span` and `gsl::at`;
  pointer arithmetic stays on, for a project to turn off with its reason.
