# Changes

What changed in each version of the template, and what a project has to do when `copier update`
brings it.

## Unreleased

Three rules of CodingStyle.md that clang-tidy now checks, by checks of our own: bits through one
class, no `friend`, and one form per operator; and a literal's suffix in lower case.

- **A literal's suffix is `u` and `L`: `192u`, `1000uL`, `1LL`** (CodingStyle T-22).
  `readability-uppercase-literal-suffix` asked for `192U`, which a hexadecimal value reads on as
  a digit; `NewSuffixes` keeps the `L` upper case, as MISRA 5.13.5 asks, and turns the `u` to
  lower case. The template's own code follows.

- **`docs_inputs()` in `tools/project.sh`:** the input folders of each run of Doxygen, one line a
  run, each into `build/docs/<n>/`, for a firmware whose backends each define a class of the same
  name, which one run would merge; a warning the runs share counts once. Without it, one run as
  before (#62).
- **`make docs` counts warnings, not lines:** one that goes on over lines of its own was counted
  once per line, and the gaps let pass could come out negative. And with no `NAMESPACE` it no
  longer speaks of `::internal`.
- **Bits through one class, H-9:** no `&`, `|`, `^`, `~`, `<<` or `>>` on an integer outside the
  bit class, nor `&=` and its like; `Bit<uint8_t>::SetBits(DDRB, _BV(DDB5))`, not
  `DDRB |= _BV(DDB5)`. The class holds once what such an expression gets wrong: the promotion to
  `int`, a shift as wide as the type, the read-modify-write of a register. A mask of named masks
  may still be or'ed in a `constexpr`. `house-bitwise-in-bit-class` checks it; it takes any class
  named `Bit` as the bit class, and leaves out the macros of a system header, as avr-libc's
  `_BV()`, and the body of a type's own bitwise operator, as a `byte`'s.
- **`Bit` in the skeleton:** Utl's class, in `src/<namespace>/Bit.h` of a library and in
  `src/Utility/Bit.h` of a firmware, with its tests and its cases of what must not compile; new
  is `Combine()`, the mask of up to four masks. SelfCheck checks it on the AVR, and SelfCheck and
  the firmware's `platform/avr/main.cpp` write their registers through it.
- **No `friend`, C-12:** but an operator defined in the class, a hidden friend.
  `house-no-friend` checks it.
- **One form per operator, C-15:** a member where it changes its operand or is unary, as `+=`,
  `++` or `-x`; a hidden friend where it is binary and leaves its operands, as `+`, `==` or `<`.
  `house-operator-form` checks it. Misra.md names the two checks at Rules 7.0.4 and 16.6.1.
- **`tests/tidy_module/system/`:** a folder of system headers for the module's cases, as
  avr-libc's are to a project.
- **Misra.md corrected:** Rule 9.5.1 is Advisory and 9.5.2 Required, not the
  other way round; 16.5.1 is Required. Checks that find only some cases of a rule are marked
  *in part*; `google-runtime-operator` does not cover 16.5.1, nor the pointer arithmetic check
  8.9.1, which are now *Review*. New in *Covered by*: `cert-err34-c`, `cert-env33-c`,
  `cert-err52-cpp`, `bugprone-unused-raii`, `misc-static-assert` and two more, all on already.
  The deviation from Rule 4.1.1 names `-fno-exceptions -fno-rtti` on the AVR.
- **The action target-setup asks for `ARG ARDUINO_CLI_VERSION` only where a target uses
  arduino-cli.** It asked always, and a firmware of CMake and PlatformIO whose Dockerfile installs
  no arduino-cli, as Wordclock's, failed every job that sets up its targets.

On the update: `make tidy` runs the three new checks, as `.clang-tidy` turns on `house-*`, and
fails where the code breaks the rules. Move each bitwise operator into the bit class, or to one
of its functions; a project with a bit class of another name sets
`house-bitwise-in-bit-class.BitClasses` in `.clang-tidy`. Make each `friend` a public function,
and each operator a member or a hidden friend as C-15 says. A project not ready yet turns a
check off in `.clang-tidy`, with the reason, until it is (S-3). `src/` stays as it is: the
skeleton's `Bit.h` comes only with `copier copy`.

And a `U` in a suffix is a finding of `make tidy` now; clang-tidy's `--fix`
writes it as `u`, or `sed -E 's/\b(0[xX][0-9a-fA-F]+|[0-9]+)U(L{0,2})\b/\1u\2/g'` over the sources.

## v0.7.5

- **`make lint` reads every shell script with shellcheck,** not those of `tools/` alone:
  `.devcontainer/llvm-headers.sh` failed the SC2312 that `.shellcheckrc` turns on, and nothing
  noticed until Wordclock's lint, which reads them all. Its last line takes the version into a
  variable first.
- **`cppcheck_flags()` in `tools/project.sh`:** further flags of cppcheck, after those of
  `tools/cppcheck.sh`, as include folders, a `--suppressions-list` or a later `--std`. A firmware
  made from the template gives `--std=c++17` there, as it is compiled as gnu++17 and cppcheck read
  it as C++11.
- **`make stack` reads a firmware linked with `-flto`, and one whose image a subfolder adds.**
  `cmake_graphs()` takes the call graphs of the link where it finds them beside the ELF,
  `<program>.elf.ltrans<N>.ltrans.ci` of `-save-temps=obj`, as the frames of the compile are those
  before LTO inlined across sources; else `CMakeFiles/<program>.dir`, wherever in the build it
  lies.
- **A build of PlatformIO fails on the project's own warnings, not on a library's.** It was
  given `-Werror` on the project's sources, which a header of a library or of the framework
  that such a source includes failed as well: Wordclock's RP2350 on `ESPAsyncWebServer`. The log
  is read for warnings in the project's own files instead, those of a hidden folder as `.pio/`
  not among them.

On the update: nothing; a firmware made before may add the `--std=c++17` of a new one.

## v0.7.4

- **The Doxygen blocks are wrapped to the project's `ColumnLimit`.** `tools/lib/doc_indent.py`
  rewrapped a command's paragraph to 80 columns whatever `.clang-format` said, and so fought
  clang-format in a project with a limit of its own: Wordclock's 100 columns turned into 160
  findings of `make lint` on blocks clang-format had left alone. It reads `ColumnLimit` from
  `.clang-format` now, 80 where it sets none.
- **`make tidy` reads a firmware of several platforms.** A target's database holds the sources
  of its own folder of `platform/`, not the others', and `tools/tidy.sh` failed on the first source
  of another target's folder. A source of `platform/` is read with the targets that compile it
  now, and one that none compiles fails, rather than going unread. `HOST_CODE` is read with the
  tests only, and `TIDY_SKIP` in `tools/project.sh` names folders a target of the project's own
  reads, as a backend's tests built by a script of their own.
- **`make headers` compiles the headers of `platform/` too.** A header of `platform/<name>/` is
  compiled on its own by each target that compiles a source of that folder, with that source's
  flags, where it lies under one of their include folders and not in a hidden one, as `.pio/`: a
  firmware's backends are compiled by their own compilers, and their headers were checked by
  none. A header of `src/` takes its flags from a source of the project's `src/`, no longer from
  any path with `/src/` in it.
- **A GCC warning or optimisation clang does not know is no error of `make tidy`.** A build that
  asks GCC for `-Wduplicated-cond`, or ESP-IDF's `-freorder-blocks`, had clang-tidy report each
  as an error of every file of the target: 373 and 56 on Wordclock. `tools/lib/clang_database.py`
  passes `-Wno-unknown-warning-option` and `-Wno-ignored-optimization-argument` now.

On the update: nothing; a project that sets no `ColumnLimit` is wrapped at 80 as before, and one
of one platform is read as before.

## v0.7.3

- **`coverage_suites()` in `tools/project.sh`:** a project's tests that `CMakeLists.txt` does not
  build, as a backend's suite compiled by a script of its own, run with `--coverage` and count
  in `make coverage` with the tests', against `COVERAGE_MINIMUM` together. Wordclock's two host
  suites reach 4.6 % of its lines that its core tests do not.
- **`make coverage` keeps the notes of a source outside its `CMakeLists.txt`'s folder.** CMake
  writes each `..` of such a path as `__`, `__/__/src/Display.cpp.gcno`, and the check for notes
  whose source is gone took them all for stale: gcov then read counts without notes and failed.

On the update: nothing.

## v0.7.2

- **`COVERAGE_MINIMUM` in `tools/project.sh`:** the percentages of lines, branches and conditions
  of `src/` that `make coverage` demands, `100 100 100` unless set. For a project brought in below
  them, as Wordclock is at 94 % of its lines: it states what it reaches and raises it as it
  reaches more, where the only choice was to leave `make coverage` out of `make check`. A
  value that is no percentage, or a fourth, fails rather than being read as 0.

On the update: nothing; a project without the setting demands 100 % as before.

## v0.7.1

Four faults of v0.7.0, found on MaxMatrix's update to it.

- **`make fuzz` links in the dev container:** it installs `libclang-rt-dev`, libFuzzer and the
  sanitizers of clang, which `clang` only recommends; CI passed, as its runner has them already.
- **`AGENTS.md` has the blank line below its title again,** which v0.7.0 dropped.
- **`make tidy` no longer warns** `could not open directory 'test/fuzz/'` in a project without
  fuzzers.
- **`make arduino-lint` passes with a file deleted and not yet staged,** as after a
  `copier update` that removes one, where `tar` failed on it.

On the update: rebuild the dev container.

## v0.7.0

`platformio`, the third build system, and a firmware built by it: the first half of E-1's step 4.

- **A target with `BUILD_SYSTEM=platformio`** is the environment `PIO_ENV` of the `platformio.ini`
  in `PIO_DIR` of `tools/project.sh`; its platform, board and framework, the Arduino core or none,
  are the `platformio.ini`'s to name and pin. `tools/lib/platformio.sh` builds it with the
  PlatformIO `uv.lock` pins, and gives it what the tools need in PlatformIO's own variables: the
  build folder per variant, the variant's flags, and `-Werror` on the project's own sources only,
  as a framework's warnings are not the project's to fix. The sizes are what pio reports; the
  database is `pio run -t compiledb`'s, moved out of the sources and rewritten for clang.
- **PlatformIO 6.2.0 in `pyproject.toml` and `uv.lock`,** pinned, as an unpinned one became 6.2.0
  under Wordclock without anybody asking. CI's `target-setup` syncs the tools and caches
  PlatformIO's platforms where a target is built by it.
- **`platformio`, a question of `copier copy` for a firmware:** yes builds the image through
  `platform/avr/platformio.ini`, `platformio/atmelavr@5.3.0` without a framework, rather than with
  CMake; the host tests stay CMake's.
- **Build systems mix in one project:** `tests/check.sh --platformio --targets` builds the
  ATmega328P by PlatformIO and the AVR128DA48 by CMake, each read by clang-tidy and its headers
  compiled by its own compiler. `make stack` adds up the second and says why not the first.
- `clang_database.py` names each file by its whole path, as PlatformIO names them relative to its
  project, and drops `PICO_PLATFORM`, a CMake variable of the Pico SDK passed on as a macro.
- `platform/avr/main.cpp` is free of warnings under avr-gcc 7 as well, which PlatformIO's
  atmelavr brings, and which warns under `-Wconversion` of `|=` on a register.

On the update: `uv sync` installs PlatformIO into the tools' environment; nothing else changes
until a target names it.

What the compiler has to warn of is tested as what it must not compile is:
`add_compile_warn_tests()` in `cmake/checks.cmake`, the cases in `test/compile_warn/`.

- **A case** does what compiles but is a mistake, as dropping a result `_NODISCARD` marks, and
  names the warning in its line `// Expected: ...`. Its test passes only if the compiler gives
  that warning, in each standard the call names: the macro may be another attribute in each.
  The case is compiled to an object with the build's warnings, as GCC reports
  `warn_unused_result` only when it generates code.
- **clang-tidy leaves the folder out,** as it does `test/compile_fail/`: a case warns on purpose
  and is in no database. MaxMatrix had to write its case into the build folder for that.
- **A new library** gets one case, a dropped `Clamp()`, in gnu++11 and C++17; a firmware the
  call, in gnu++17, and no case.

On the update: `CMakeLists.txt` is the project's, so a project that wants such tests adds the
call itself, as the template's `CMakeLists.txt` has it, and puts its cases in
`test/compile_warn/`.

`_NODISCARD` is `[[nodiscard]]` wherever the compiler has it, chosen by `__has_cpp_attribute`
rather than by `__cplusplus` (CodingStyle FN-10).

- **Why:** GCC, clang and avr-gcc 7.3 have `[[nodiscard]]` in C++11 already, and GCC's
  `warn_unused_result`, which the macro was before C++17, made every example of MaxMatrix 10 to
  30 bytes of flash larger on the Uno; with `[[nodiscard]]` they are as without the marks, to
  the byte. clang reports the attribute before C++17 only under `-Wpedantic`, which the template
  does not turn on.
- **A cast to `void`** now silences the warning on every compiler the template builds with, so
  none reports one any more; FN-10 keeps `Discard()` all the same, as a cast does not say that
  the drop is meant, and the macro still falls back to `warn_unused_result` where a compiler
  lacks the attribute.

On the update: nothing to do; a result dropped through a cast that avr-gcc reported before no
longer fails, so review looks for one (ROADMAP C-3).

`make coverage` fails unless the tests reach every line, branch and condition of `src/` (C-7, idea
I-12 before).

- **Conditions, MC/DC:** with GCC 14 and later the coverage build adds `-fcondition-coverage`,
  which shows whether each condition of a decision turns its outcome alone; gcovr counts them
  beside the lines and branches. Another compiler gets a warning when CMake configures: the
  conditions are then not checked.
- **A figure that fails:** TS-4 of `docs/Testing.md` asked for every line and branch, and review
  checked it; now gcovr fails below 100 % of each. An exclusion stays as it was,
  `GCOVR_EXCL_LINE: ` and its reason, and `GCOVR_EXCL_BR_LINE: ` takes the conditions of its
  line along with the branches.
- **What was missed:** the terminal lists the lines of each file, and of the branches; gcovr
  names no line for a condition, as it counts one as reached once either of its outcomes is, so
  the HTML report in `build/coverage/html/` marks those.

On the update: `make coverage`, and so `make check`, fails where the tests miss something they
missed before without a word. MaxMatrix, measured first, missed 6 lines, 9 branches and 11 of
394 conditions, and closed them before taking this.

`make fuzz`: fuzzers on libFuzzer, under ASan and UBSan, part of `make check` and of CI (I-10).

- **A fuzzer is `test/fuzz/<name>_fuzz.cpp` or `<Class>Fuzz.cpp`,** defining
  `LLVMFuzzerTestOneInput()`. `add_fuzzers()` of `cmake/checks.cmake` builds each against the
  library of the default variant when `<PREFIX>_FUZZ` is on, which the new preset `fuzz` turns on,
  by clang into `build/fuzz/`; the library is instrumented as well, as libFuzzer steers by the
  coverage of what it reaches.
- **`tools/fuzz.sh` runs them side by side, `FUZZ_SECONDS` each, 60 if not set,** each in a folder
  of its own below `build/fuzz/work/`, going on from the corpus the last run left, and starting
  from the seeds in `test/fuzz/corpus/<Name>/`, with `<Name>.dict` and `<Name>.options` beside the
  source where there are. What failed is named with the file that replays it.
- **CI:** a job `fuzz` in `static-analysis.yml`, which keeps the corpus between runs and uploads
  what failed; clang-tidy reads the fuzzers with the database of the preset `fuzz`.
- **The skeleton has one:** `clamp_fuzz.cpp` for a library, `BlinkerFuzz.cpp` for a firmware, with
  a `test/fuzz/.clang-tidy` that lets a fuzzer cut its input by offsets. Testing.md has the rules
  TS-24 to TS-27.

On the update: `make check` runs `make fuzz`, which has nothing to run until the project adds
`add_fuzzers(DIRECTORY ${CMAKE_SOURCE_DIR}/test/fuzz LIBRARY <namespace>_default)` to its
`CMakeLists.txt`, which the update does not touch, and a fuzzer to `test/fuzz/`, with the
`.clang-tidy` of the skeleton's beside it.

`make stack`: the deepest stack of every program on an AVR, against the RAM it leaves free (W-11).

- **`tools/avr_stack.py`, out of Wordclock's `avr-stack.py`:** the deepest path from `main()` and
  the static constructors, plus the deepest interrupt handlers that can be on the stack at once,
  from the call graphs avr-gcc writes beside each object, and library code by its disassembly;
  against the part's RAM less `.data` and `.bss`. It refuses recursion, a call through a pointer,
  a frame of dynamic size and `malloc()`, each of which would make the sum a lower bound.
- **Made general for any AVR:** a return address of three bytes on a part with more than 128 KiB
  of flash, as the ATmega2560; one handler at a time on a classic AVR, and a handler that sets
  the interrupt flag refused, two on the megaAVR 0 and Dx series; the RAM from `RAMSTART` and
  `RAMEND`, which every part's header defines.
- **A call into libgcc the graph does not list is counted:** avr-gcc calls its helpers, as
  `__udivmodsi4` for a division of 32 bits, after it wrote the graph, so each function's own
  instructions are read for them. Wordclock's tool, which lacks this, comes out one byte short on
  its own image: 628 bytes against 629, through `__udivmodqi4` below `SetPixel`.
- **`cmake/toolchains/avr-gcc.cmake` compiles with `-fstack-usage -fcallgraph-info=su`,** which
  change no byte of the code; `tools/lib/clang_database.py` drops the second for clang.
- `tools/stack.sh` walks every program of every AVR target CMake builds, in each variant, and
  says so where there is none: arduino:avr's avr-gcc 7.3 writes no call graph.

On the update: `make check` runs `make stack` as well; for an Arduino library it has nothing to
add up and says so.

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
