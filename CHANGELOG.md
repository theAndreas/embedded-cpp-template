# Changes

What changed in each version of the template, and what a project has to do when `copier update`
brings it.

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
