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

### M5 - A generic embedded library on the ETL

| Id | Item | Status |
|---|---|---|
| E-1 | A second flavor of the template, chosen by a copier question: CMake first, the library for any target, Arduino as one user of it | open |
| E-2 | A CI matrix of C++11, 14, 17 and 20, GCC and Clang, on the host, plus avr-gcc through arduino-cli | open |
| E-3 | The [ETL](https://github.com/ETLCPP/etl) (MIT) as a pinned dependency: CMake for the host, `etl-arduino` for Arduino. A library of our own on top for what it lacks or does otherwise: errors as `Result` with groups, text and tables in flash. Recommended over a fork, which would have to follow a very active upstream; a fix goes upstream | idea |
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

- **copier, not a GitHub template**: a GitHub template is copied once and then drifts; copier
  updates a project as a three-way merge.
- **The template's files under GPL-3.0-or-later**, as Utl's were; a project picks its own licence.
- **cert-\* without its aliases**: on Utl it reported only what bugprone- checks report already,
  each wanting a second NOLINT.
- **The bounds profile of the Core Guidelines off**: it wants `gsl::span` and `gsl::at`;
  pointer arithmetic stays on, for a project to turn off with its reason.
