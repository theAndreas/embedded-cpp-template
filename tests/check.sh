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
#   tests/check.sh [--keep] [--targets] [CHECK...]
#
# CHECK is a target of the project's Makefile, as lint, tests, clang, tidy, headers or
# self-check; all of what `make check` runs if none is given. With --keep the project is left in
# build/Demo for a look, else it is removed. With --targets the project is built for two targets
# rather than one, an Uno and a Mega, as tools/project.sh describes a second target: the same
# core, so that it costs no download, and each run in simavr. The folder is named after the project, as cpplint
# takes the header guards from it. The url is the template's, as arduino-lint checks that it
# answers.

set -euo pipefail

TEMPLATE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly TEMPLATE_DIR
readonly ALL_CHECKS=(lint arduino-lint tests clang sanitize coverage tidy cppcheck headers docs
  examples sizes self-check)
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

# What tools/project.sh of the project gets with --targets, after what the template wrote: the
# text of a function, expanded by the project's shell, not this one.
# shellcheck disable=SC2016
readonly TWO_TARGETS='
TARGETS=(uno mega)
target_settings() {
  case "$1" in
    mega)
      FQBN="arduino:avr:mega"
      SIMAVR_MCU="atmega2560"
      ;;
  esac
}'

main() {
  local keep=0
  local targets=0
  while [[ "${1:-}" == --* ]]; do
    case "$1" in
      --keep) keep=1 ;;
      --targets) targets=1 ;;
      *)
        err "Usage: $0 [--keep] [--targets] [CHECK...]"
        exit 2
        ;;
    esac
    shift
  done
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
  if ((targets)); then
    echo "${TWO_TARGETS}" >> tools/project.sh
  fi
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
