# Changes

What changed in each version of the template, and what a project has to do when `copier update`
brings it.

## Unreleased

A project can be a firmware: `kind: firmware` makes an image of its own for each target rather
than a library with examples, the third step of E-1.

- **`kind`, a question of `copier copy`:** library, the default, or firmware. A firmware has its
  portable code in `src/`, a folder per module, tested on the host, and what each target needs
  beside it in `platform/<name>/`; `PLATFORM` in `CMAKE_DEFINES` of `tools/project.sh` says
  which. It is built by CMake with a toolchain file as gnu++17, as `arduino` is a library's
  question; its skeleton is a module, `Blinker`, its test, and `platform/avr/main.cpp`, which
  counts milliseconds with a timer of a classic AVR or of the megaAVR 0 and Dx series. No
  SelfCheck: an image runs as long as the part has power.
- **What a target builds is named by its build system:** `target_programs` prints the examples
  of a library or the image of a firmware; CMake writes them into `programs.txt` of its build
  folder, and the ELF files go into `programs/` rather than `examples/`. The table of sizes is
  headed `Image` for a firmware, still `Example` for a library.
- **`platform/` is read as `examples/` is:** formatted and checked by `make lint`, read by
  clang-tidy with the target's flags and by clangd through `build/tidy/clangd`, with a
  `.clang-tidy` of its own that lets an interrupt routine share a global and a register take the
  datasheet's numbers.
- copier refuses `arduino=true` for a firmware.
- **The SelfCheck of a library of CMake alone passes clang-tidy on a Dx part:** #37's compiled
  there, but clang-tidy reported `<avr/sleep.h>`, which clang does not know for a Dx part, a
  register read through avr-libc's union, and `<avr/sfr_defs.h>` unused. It now sleeps only where
  simavr runs it, on a classic AVR, writes the flat `USART0_BAUD` and its bits without `_BV`.
  `tests/check.sh --no-arduino --targets` builds and reads it for an AVR128DA48 as a third target.

On the update: nothing for a library.

A library need not be an Arduino library: `arduino: no` makes one of C++ and CMake alone, built for
its targets by avr-gcc with a toolchain file, the second step of E-1.

- **`arduino`, a question of `copier copy`:** yes, the default, makes the Arduino library it always
  made. No makes a library without `library.properties`, without the mock of the core in
  `test/mock/`, and with its examples as programs, `examples/<Name>/<Name>.cpp` with a `main()`;
  SelfCheck writes to the first USART itself, of a classic AVR as simavr runs it or of the
  megaAVR 0 and Dx series. copier then asks for the part and its clock, `mcu` and `f_cpu`, in
  place of the board and the core.
- **`cmake`, the second build system:** a target with `BUILD_SYSTEM=cmake` is `CMakeLists.txt`
  configured with the toolchain file `CMAKE_TOOLCHAIN` and the definitions `CMAKE_DEFINES` of
  `tools/project.sh`, one build folder per variant, the variant's flags in `VARIANT_FLAGS`, as
  MinSizeRel and with warnings as errors. Cross-compiled, `CMakeLists.txt` builds each example as
  `examples/<Name>.elf`, the library compiled into it. The template brings
  `cmake/toolchains/avr-gcc.cmake`, for avr-gcc and avr-libc from apt, which
  `.devcontainer/apt-packages.txt` names for such a library.
- **clang-tidy reads a CMake target as the target:** `tools/lib/clang_database.py`, out of
  Wordclock's `cross-clang-tidy.py`, rewrites GCC's database for clang, with the target, the
  compiler's own include folders and without GCC's own flags. `make headers` compiles each header
  with the target's own compiler, from the database CMake wrote.
- **What a build system does is a function of its own:** `target_setup`, `target_build`,
  `target_elf`, `target_sizes` and `target_database` in `tools/common.sh` call those of
  `tools/lib/arduino_cli.sh` or `tools/lib/cmake.sh`; `make examples` fails as well where a build
  that passed left no ELF file of an example.
- **CI follows the build systems:** the action `arduino-setup` is now `target-setup`, which
  installs arduino-cli only where a target is built by it (`tools/setup.sh --systems`).
  arduino-lint runs where there is a `library.properties`, and `release.yml` takes the version
  from `project()` of `CMakeLists.txt` where there is none.

On the update: an Arduino library has nothing to do; its workflows name `target-setup` after the
update, and `.github/actions/arduino-setup/` goes.

A project is built for one target or several, each with its build system: the first step of E-1,
which brings CMake and PlatformIO as build systems beside arduino-cli, and firmware beside the
library.

- **`TARGETS` in `tools/project.sh`:** the targets a project is built for, each with its own
  board, core and simavr MCU through `target_settings()`, and its `BUILD_SYSTEM`, of which
  arduino-cli is the first. `make setup`, `examples`, `self-check`, `tidy`, `cppcheck` and
  `headers` walk them all. A project that names no target has one, `board`, with the settings it
  always had, and nothing changes for it: its table of sizes keeps its columns, and a target
  name joins them only where there are two.
- **The arduino-cli functions in `tools/lib/arduino_cli.sh`:** `compile_example`, `elf_of`,
  `avr_gcc_bin_dir` and `list_examples` keep their names, so a `check_elf()` that calls them, as
  Utl's does, works as before; `select_target()` has set FQBN and the rest for the target.
- **New places in `build/`:** a target's builds in `build/targets/<target>/<variant>/`, its
  compilation database in `build/tidy/targets/<target>/<variant>/`. clangd reads the examples'
  through `build/tidy/clangd`, which points to the first target's, and `make headers` takes the
  list of databases from `build/tidy/databases`.
- **`tests/check.sh --targets`** makes the project for an Uno and a Mega, one core and both run
  in simavr, and the template's CI checks it beside the project of one target.

On the update: nothing to do. A script of the project's own that reads `build/<variant>/` or
`build/tidy/default/` reads the new places; one that sources `tools/common.sh` finds the first
target selected.

## v0.6.1

- **`house-braced-var-init` leaves a text in a C array alone:** `char const kName[] = "font";`
  keeps its `=`, as T-20 writes it. v0.6.0 reported it, and its fix-it, `kName[]{"font"}`, was
  then reported by `modernize-avoid-c-arrays`, whose `AllowStringArrays` lets only the `=` form
  through.

On the update: a project that took v0.6.0's fix-its for such arrays writes them with `=` again,
or `make tidy` reports them under `modernize-avoid-c-arrays`.

## v0.6.0

Braced initialization (CodingStyle.md T-3) is checked everywhere, not only in a constructor's
member initializer list.

- **`house-braced-var-init`, a second check of our own:** a variable or a data member's default
  written with `=` or parentheses is reported: local, global, `static`, a static data member, the
  variable of a `for` or of a condition, and with `auto` as well. Where braces keep the meaning,
  its fix-it writes them: `int count = 0;` becomes `int count{0};`, `Point p = {1, 2};` and
  `Point p = Point(1, 2);` become `Point p{1, 2};`. It reports without a fix-it where an argument
  narrows, where the type is a template's parameter, and where `T x = value;` initializes a class
  with an `explicit` constructor, which braces would consider; and not at all where braces would
  choose a `std::initializer_list` constructor, or for `auto list = {1, 2};`. A parameter's
  default, the variable of a range-based `for`, a structured binding and a lambda's capture are
  left alone. It holds C++11 and C++17 alike, whose trees of the same initialization differ.

On the update: `make tidy` reports every initialization with `=` or parentheses that a project
has. Most have a fix-it, which `clang-tidy --fix` applies with the module loaded; the rest want
the conversion written out, or a `NOLINT` naming T-3 and the reason.

## v0.5.2

`make check` takes minutes fewer: the tests run side by side, and clang-tidy reads one file per
core.

- **The tests run side by side:** the Makefile exports `CTEST_PARALLEL_LEVEL`, the number of
  cores unless the environment sets it, so that ctest runs as many tests at a time as there are
  cores, in every target that runs it. A project's tests wait on the clock and on ports as much
  as on a core: MaxMatrix's 1646 took 10 s rather than 94, each time `make check` runs them. A
  test that needs something to itself, as a port, says so with `RESOURCE_LOCK`.
- **clang-tidy reads one file per core:** `tools/tidy.sh` runs it through `run-clang-tidy.py`,
  which the pinned clang-tidy brings and which prints the findings of a file together. It reads
  every file of the database it is given, so `tidy.sh` writes one of the files to read alone,
  in `build/tidy/selected/`, and a file the database lacks still fails rather than going
  unread. In MaxMatrix, `make tidy` took 387 s rather than 529: the rest was one test file of
  2000 lines, which clang's static analyzer read for two minutes in each variant. Split into a
  file per header, as TS-5 asks (MaxMatrix #83), it took 130 s.

On the update: nothing to do, unless the tests of a project share something without saying so,
as a port or a file: they get a `RESOURCE_LOCK` first, or fail now and then.

## v0.5.1

A project's CI gets its Ubuntu packages when the runners' mirrors do not answer: on 2026-11-11
they reached no server of Canonical for hours, and every job that installed from apt failed.

- **apt-get update moves off a mirror that does not answer:** `.github/actions/apt-install`
  takes Azure's mirror out of the runner's list from the second attempt at `apt-get update`, as
  it did for the downloads already, so that apt goes on to archive.ubuntu.com, and the third
  attempt goes to mirrors.edge.kernel.org, one of Ubuntu's listed mirrors that Canonical does
  not run; apt checks the archive's signature whichever mirror serves it. On 2026-11-11 the
  runners reached no server of Canonical for hours, and every CI job that installed from apt
  failed before it built anything; kernel.org's mirror answered at once. The template's own CI
  installs through the action now, so that it tries a change to it first.

On the update: nothing to do. `.github/actions/apt-install` comes from the template, and
`copier update` replaces it unless the project changed it.

## v0.5.0

The scripts in `tools/`: in order, one language fewer, the indenter of the Doxygen blocks
tested, and a failure no longer hidden in a list of files. And what MaxMatrix, the second project
to come onto the template, needs of it: code of its own on the host checked as the library is,
and clang-tidy reading each variant with the libraries it includes.

- **clang-tidy reads a variant with the libraries it includes:** `tools/tidy.sh` has arduino-cli
  write the board's compilation database once for each variant, with its defines, rather than
  once for all with the defines added after. arduino-cli finds the libraries a sketch needs by
  its includes, and one that only a variant includes, as MaxMatrix's SPI under
  `MAXMATRIX_USE_SPI`, had no include folder in the database: clang-tidy stopped at
  `'SPI.h' file not found`.
- **Code of the project's own on the host:** `HOST_CODE` in `tools/project.sh`, the folders of
  code that runs on the host beside the library, as a simulator or a tool, built by
  `CMakeLists.txt`. `make lint` formats and checks them as `src/`, and `make tidy` reads them
  with the tests. A name that is no folder fails both. Empty by default.
- **`tools/` holds what `make` calls, `tools/lib/` what those scripts call:** `doc_indent.py`,
  which `lint.sh` runs, and `tidy_module.sh` with the module's sources in `tidy_module/`, which
  `tidy.sh` builds. `tools/README.md` says what each file is, and when a script is shell or
  Python. `common.sh` stays where it is, as a project's own scripts source it.
- **`tools/lib/doc_indent.py` instead of `tools/doc_indent.pl`:** the scripts are shell, which
  drives the tools, and Python, which reads and writes data; the Perl script was the one that
  nothing checked. ruff reads it now, and the template needs Perl no more. It indents what the
  Perl script indented, compared on every source and on 3000 generated blocks, with four
  exceptions, each a fix:
  - the width is counted in characters, as clang-format counts it, not in bytes: a line with
    umlauts that fits 80 columns is no longer broken;
  - a block ends where `*/` stands, also after the text of a line; before, the block went on to
    the next line that started with `*/`, and so took in the comments after it;
  - an example, `\code`, that is not closed ends with its block; before, the blocks after it
    were taken for code and left as they were;
  - a backtick that opens no code span stays; it was dropped, as was the closing one of a
    `\param` whose name is a code span.
- **Its cases:** `tests/doc_indent/`, which `tests/doc_indent.py` checks in the template's CI:
  `--fix` makes the expected file of each, `--check` fails exactly where `--fix` changes
  something, and a second `--fix` changes nothing.
- **shellcheck's check of masked returns:** `.shellcheckrc` turns on
  `check-extra-masked-returns` (SC2312). `mapfile -t files < <(git ls-files ...)` never saw git
  fail: the list was empty, and the check over it passed. A list of files comes from
  `list_files` in `tools/common.sh` now, which fails where git fails or no file matches; the
  output of every other command goes into a variable before it is used.
- **The template's own Python scripts are read by ruff** in its CI, with the rules of
  `pyproject.toml`.

On the update: a file of the project's own that names `tools/tidy_module.sh`,
`tools/tidy_module/` or the indenter by its old path names `tools/lib/` now; the template's files
move with the update. A hook in `tools/project.sh` that reads a command's output through
`< <(...)` or `$(...)` as an argument is reported by `make lint` now; give the output a variable
first, as the template's scripts do. A project that added exceptions of its own at the end of the
`Checks` of `.clang-tidy` gets a conflict there, as the note on `house-*` below them names the new
path: keep the project's lines and the template's note.

## v0.4.0

- **Ubuntu packages of the project's own:** `.devcontainer/apt-packages.txt`, one or more per
  line, which the Dockerfile installs into the dev container. The action `apt-install` takes it
  as `file:`, beside `packages:`, and the jobs of `ci.yml` and `static-analysis.yml` that build
  with CMake install it as well, so that CI has what the container has. The template writes the
  file once, empty; it is the project's, and an update leaves it alone.
- **Settings of the reference of the project's own:** `Doxyfile.project`, which `Doxyfile`
  includes at its end, so that `INPUT +=` adds a page or `PREDEFINED +=` the switch under which
  more of the API is compiled. The template writes it once; it is the project's.
- **Coverage of a target in a subdirectory:** `tools/coverage.sh` took the notes of a target that
  `add_subdirectory()` adds, as MaxMatrix's control program, for those of a deleted source, as it
  looked for the source in the root; it removed them, and gcovr failed on the counts left
  without. It finds the source in its subdirectory now.

On the update: rebuild the dev container. A project that installed packages of its own in a
workflow of its own can list them in `apt-packages.txt` and pass `file:` instead.

## v0.3.1

What the first update to v0.3.0, Utl's, found in the limits of a function's size.

- **The cognitive complexity of a test leaves GoogleTest's macros out:** `EXPECT_` and `ASSERT_`
  expand to an `if` and an `else` each, so a test without a branch of its own counted 20 and
  failed the limit of 15. `test/.clang-tidy` sets `IgnoreMacros`; a test's own branches still
  count.
- **SelfCheck has no limit of lines, as a test has none:** `examples/SelfCheck/.clang-tidy`. It is
  the test on the target, and a check of many cases is long but flat; its parameters, nesting and
  complexity keep the library's limits.

On the update: `test/` and `examples/SelfCheck/` are the project's, and an update leaves them
out. Take `test/.clang-tidy` and `examples/SelfCheck/.clang-tidy` from a new copy; this holds for
the update to v0.3.0 as well, whose `LineThreshold: none` for tests is in that `test/.clang-tidy`.

## v0.3.0

How the code is written and tested, for a person and for a coding agent; checks of our own as a
clang-tidy module; the README's sizes checked against the build.

- **The README's table of flash and RAM checked against the build:** `make sizes`, part of
  `make check` and of CI, compares the table between `<!-- sizes -->` and `<!-- /sizes -->` in
  README.md with the one the last `make examples` measured, byte for byte, as CI and the dev
  container compile with the same pinned core; `make sizes FIX=1` writes it in.
  `tools/compile_examples.sh` writes its table to `build/sizes.md` when every example compiles.
  A README without the block has nothing to check.
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

On the update from v0.2.1:

- Rebuild the dev container, for LLVM's headers and for gh's sign-in.
- `make tidy` may now report what passed before. A member initializer written in parentheses:
  `tools/tidy.sh` applies nothing, but clang-tidy's `--fix` swaps the parentheses where it says
  so. A function that is too large: split it, or suppress the finding with its reason.
- `CMakeLists.txt` is the project's and is not updated: to build a module in a folder of its own,
  take its two `file(GLOB_RECURSE ...)` lines and the `list(FILTER ...)` from a new copy.
- Notes of the project's own for an agent go at the end of `AGENTS.md`.
- A table of flash and RAM in the README is checked once it stands between `<!-- sizes -->` and
  `<!-- /sizes -->`; `make examples` and `make sizes FIX=1` write the measured one in.

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
