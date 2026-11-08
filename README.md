# Template for embedded C++

The template of an Arduino library on the AVR, as [Utl](https://github.com/theAndreas/Utl)
grew it: its style, its static analysis, its tests on the host and in simavr, its CI and its
dev container. A project is made from it with [copier](https://copier.readthedocs.io) and kept up
to date with it: every improvement made here reaches each project as a merge, not as a copy by
hand.

## What a project gets

| Area | What | Where |
|---|---|---|
| Style | The Google C++ style with `const` on the right, braced initialization, Doxygen blocks | `.clang-format`, `CPPLINT.cfg`, `docs/Documentation.md` |
| Static analysis | clang-tidy with the C++ Core Guidelines, CERT, bugprone, the analyzer and Google's names, every exception with its reason; cppcheck beside it | `.clang-tidy`, `tools/tidy.sh`, `tools/cppcheck.sh` |
| Warnings | `-Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Werror`, on the host and on the board | `cmake/checks.cmake`, `tools/compile_examples.sh` |
| Tests | GoogleTest on the host against a mock of the core, also under ASan and UBSan, with coverage; tests of what must not compile; a self check on the board in simavr | `CMakeLists.txt`, `test/`, `tools/self_check.sh` |
| Documentation | Doxygen, failing on a gap, published on GitHub Pages | `Doxyfile`, `tools/docs.sh` |
| CI | Every check above on each pull request, through the same scripts as in the container | `.github/workflows/` |
| Dev container | Every tool, pinned with its checksum; clangd with the databases of the host and the board | `.devcontainer/` |

The tools fetch their checkers at fixed versions through `uvx`, so that the container, CI and
every project check alike.

## Making a project

In a dev container of any project made from the template, or wherever `uv` is installed:

```bash
uvx copier copy gh:theAndreas/embedded-cpp-template MaxMatrix
```

copier asks for the name, the namespace, the board and the rest, and says what to run next. The
folder has to be named as the project: cpplint takes the header guards from it.

## Keeping it up to date

```bash
uvx copier update            # to the newest version of the template
uvx copier update --vcs-ref v0.2.0
```

copier remembers in `.copier-answers.yml` which version a project was made from. An update
renders that version and the new one, and applies the difference to the project as a three-way
merge: what the project changed itself stays, and a conflict is marked in the file as git marks
one. Commit before an update, review the diff after it, and run the checks.

## What is the template's and what is the project's

- **The template's**: the configuration, `tools/` except `project.sh`, `cmake/checks.cmake`, the
  workflows in `.github/`, the dev container and `docs/Documentation.md`. An update brings every
  change to them. A project may still change one: add its own exceptions at the end of a list in
  `.clang-tidy`, with the reason, and the merge keeps them. A job of its own goes into a workflow
  of its own.
- **The project's**, written once and never touched by an update: `README.md`,
  `CMakeLists.txt`, `library.properties`, `tools/project.sh`, `src/`, `test/` and the examples
  `Basics` and `SelfCheck`. `copier.yml` lists them in `_skip_if_exists`.

`tools/project.sh` is how the template's tools learn about the project: the board, the core, the
build variants and their flags, and hooks such as `prepare()`, which builds what compiling needs
beyond the core. Its comment lists every setting.

## Bringing in an existing project

A project that was not made from the template has no `.copier-answers.yml` yet. On a branch of
its own:

```bash
uvx copier copy --overwrite --data project_name=Utl gh:theAndreas/embedded-cpp-template .
git diff                     # what the template would change
```

`--overwrite` replaces the template's files; the project's are left alone where they exist. Take
over what is better, restore what the project needs to keep, move what is the project's into
`tools/project.sh`, and commit with `.copier-answers.yml`. From then on `copier update` works.

## Changing the template

1. Change the files in `template/`. A file ending in `.jinja` is rendered with the answers of
   `copier.yml`; any other is copied as it is.
2. `tests/check.sh` makes a project from the working tree and runs all of its checks. A single
   one runs faster: `tests/check.sh lint tests`.
3. Add what changed to `CHANGELOG.md`, with what a project has to do on the update.
4. Merge, and tag the version: `git tag v0.2.0 && git push --tags`. `copier update` takes the
   newest tag.
