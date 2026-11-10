# Template for embedded C++

The template of an embedded C++ library, usable from Arduino as well: its style, its static
analysis, its tests on the host and on the target, its CI and its dev container. A project is
made from it with [copier](https://copier.readthedocs.io) and kept up to date with it: every
improvement made here reaches each project as a merge, not as a copy by hand.

The library is C++11 without a standard library, so that avr-gcc compiles it as well as any
other compiler. Today the template checks it on the host, by GCC and by clang, and on an AVR
board through arduino-cli and simavr; other targets and standards are next, see
[ROADMAP.md](ROADMAP.md).

## What a project gets

| Area | What | Where |
|---|---|---|
| One way in | A Makefile that names every build and check: `make` lists them, `make check` is what CI runs, `make lint` the checks of seconds | `Makefile`, `tools/project.mk` |
| Style | The Google C++ style with `const` on the right, braced initialization, Doxygen blocks | `.clang-format`, `CPPLINT.cfg`, `docs/Documentation.md` |
| Static analysis | clang-tidy with the C++ Core Guidelines, CERT, bugprone, the analyzer, Google's names and clang's warnings beyond `-Wall -Wextra`, every exception with its reason, and checks of our own (`house-*`) for the rules no check of clang-tidy's holds; cppcheck beside it; every header compiled on its own | `.clang-tidy`, `tools/`, `tools/lib/tidy_module/` |
| Warnings | `-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow`, errors in CI and under `make check` | `cmake/checks.cmake`, `CMakePresets.json` |
| Tests | GoogleTest on the host, by GCC and by clang, against a mock of the Arduino core, also under ASan and UBSan, with coverage; tests of what must not compile; a self check on the board in simavr | `CMakeLists.txt`, `test/`, `examples/SelfCheck` |
| Documentation | Doxygen, failing on a gap, published on GitHub Pages | `Doxyfile`, `tools/docs.sh` |
| CI | Every target of `make check`, in `ci.yml` and `static-analysis.yml`, each with a badge; a release from a tag | `.github/` |
| Tools | The Python tools pinned in `uv.lock`, the same in the container and in CI | `pyproject.toml`, `uv.lock` |
| Dev container | Every tool, pinned with its checksum; clangd with the databases of the host and the board | `.devcontainer/` |

## Making a project

In a dev container of any project made from the template, or wherever `uv` is installed:

```bash
uvx copier copy gh:theAndreas/embedded-cpp-template LedMatrix
cd LedMatrix && git init && git add -A && git commit -m "LedMatrix: from the template"
make setup && make check
```

copier asks for the name, the namespace, the board and the rest. The folder has to be named as
the project: cpplint takes the header guards from it.

## Keeping it up to date

```bash
uv run copier update            # to the newest version of the template
uv run copier update --vcs-ref v0.2.0
```

copier remembers in `.copier-answers.yml` which version a project was made from. An update
renders that version and the new one, and applies the difference to the project as a three-way
merge: what the project changed itself stays, and a conflict is marked in the file as git marks
one. Commit before an update, review the diff after it, and run `make check`.

## What is the template's and what is the project's

- **The template's**: the configuration, the `Makefile`, `tools/` except `project.sh` and
  `project.mk`, `cmake/checks.cmake`, `pyproject.toml` and `uv.lock`, the workflows and actions
  in `.github/`, the dev container and `docs/Documentation.md`. An update brings every change
  to them. A project may still change one: add its own exceptions at the end of a list in
  `.clang-tidy`, with the reason, and the merge keeps them. A job of its own goes into a
  workflow of its own.
- **The project's**, never touched by an update: `README.md`, `CMakeLists.txt`,
  `library.properties`, `tools/project.sh` and `tools/project.mk`, written where they do not
  exist yet; and the starting point of a new library, `src/`, `test/` and the examples `Basics`
  and `SelfCheck`, written only by `copier copy` with the answer `skeleton`. An update neither
  brings back what a project deleted of it nor merges into what it changed.

`tools/project.sh` is how the template's tools learn about the project: the board, the core, the
build variants and their flags, and hooks such as `prepare()`, which builds what compiling needs
beyond the core. `tools/project.mk` holds the project's own targets of the Makefile; one added to
`PROJECT_CHECKS` there is part of `make check`.

The template's Dependabot keeps its actions and `uv.lock` current; a project gets those updates
with `copier update`, not with a Dependabot of its own, so that the two never conflict.

## Bringing in an existing project

A project that was not made from the template has no `.copier-answers.yml` yet. On a branch of
its own:

```bash
uvx copier copy --overwrite --data project_name=LedMatrix --data skeleton=false \
  gh:theAndreas/embedded-cpp-template .
git diff                     # what the template would change
```

`--overwrite` replaces the template's files; the project's are left alone where they exist, and
`skeleton=false` leaves out the starting point of a new library. Take over what is better,
restore what the project needs to keep, move what is the project's into `tools/project.sh` and
`tools/project.mk`, and commit with `.copier-answers.yml`. From then on `copier update` works.

## Changing the template

The repository has a dev container of its own, built from the template's Dockerfile, so that it
carries exactly what a project's does: see [.devcontainer/README.md](.devcontainer/README.md).

1. Change the files in `template/`. A file ending in `.jinja` is rendered with the answers of
   `copier.yml`; any other is copied as it is.
2. `tests/check.sh` makes a project from the working tree and runs all of its checks. A single
   one runs faster: `tests/check.sh lint tests`.
3. Add what changed to `CHANGELOG.md`, with what a project has to do on the update.
4. Merge, and tag the version: `git tag -a v0.2.0 && git push origin v0.2.0`. `copier update`
   takes the newest tag.
