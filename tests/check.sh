#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Makes a project from the template, as its working tree is, and runs the project's checks on
# it: what CI would run on a project that has changed nothing yet. A template change passes
# only if a project made from it passes; this is that check, for a person and for CI alike.
#
# Usage:
#   tests/check.sh [--keep] [CHECK...]
#
# CHECK is a target of the project's Makefile, as lint, tests, clang, tidy, headers or
# self-check; all of what `make check` runs if none is given. With --keep the project is left in
# build/Demo for a look, else it is removed. The folder is named after the project, as cpplint
# takes the header guards from it. The url is the template's, as arduino-lint checks that it
# answers.

set -euo pipefail

TEMPLATE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly TEMPLATE_DIR
readonly ALL_CHECKS=(lint arduino-lint tests clang sanitize coverage tidy cppcheck headers docs
  examples self-check)
# copier from PyPI through uvx, at a fixed version, like the tools of a project.
readonly COPIER="copier==9.19.1"
# Global, as the trap that removes it runs after main() has returned.
readonly PROJECT_DIR="${TEMPLATE_DIR}/build/Demo"

err() {
  echo "$*" >&2
}

# Runs one check in the project's folder: a target of its Makefile, with warnings as errors, as
# CI runs it.
run_check() {
  STRICT=1 make "$1"
}

main() {
  local keep=0
  if [[ "${1:-}" == "--keep" ]]; then
    keep=1
    shift
  fi
  local checks=("$@")
  if ((${#checks[@]} == 0)); then
    checks=("${ALL_CHECKS[@]}")
  fi

  rm -rf "${PROJECT_DIR}"
  mkdir -p "$(dirname "${PROJECT_DIR}")"
  uvx "${COPIER}" copy --quiet --defaults --vcs-ref HEAD \
    --data project_name=Demo \
    --data "description=A library made from the template, to check it." \
    --data "url=https://github.com/theAndreas/embedded-cpp-template" \
    "${TEMPLATE_DIR}" "${PROJECT_DIR}"
  if ((!keep)); then
    trap 'rm -rf "${PROJECT_DIR}"' EXIT
  fi

  # A project is a repository: the tools list its files with git.
  cd "${PROJECT_DIR}"
  git init --quiet
  git add --all
  git -c user.name=template -c user.email=template@localhost commit --quiet \
    --message "Demo: from the template"

  local failed=()
  local check
  if [[ " ${checks[*]} " =~ \ (tidy|headers|examples|self-check)\  ]]; then
    make setup >/dev/null
  fi
  for check in "${checks[@]}"; do
    echo "=== ${check}"
    if ! run_check "${check}"; then
      failed+=("${check}")
    fi
  done
  if ((${#failed[@]} > 0)); then
    err "Failed: ${failed[*]}"
    exit 1
  fi
  echo "All passed: ${checks[*]}"
}

main "$@"
