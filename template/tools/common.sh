#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Shared by the scripts in tools/: where things lie, which targets the project is built for, and
# how a target is selected. The defaults below are what a project gets that says nothing else;
# tools/project.sh, sourced after them, is where it says otherwise. This file comes from the
# template and is brought up to date with `copier update`; what is the project's goes into
# tools/project.sh.
#
# A target is a board or chip the project is compiled for, by one build system: today
# arduino-cli, whose functions are in tools/lib/arduino_cli.sh. A script that works on the
# targets walks TARGETS and calls select_target() for each, which sets the settings of
# TARGET_SETTINGS to that target's; a project of one target need not name it.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly ROOT_DIR
readonly BUILD_DIR="${ROOT_DIR}/build"

# The defaults, each described in tools/project.sh.
# shellcheck disable=SC2034  # used by the scripts that source this file
{
  NAMESPACE=""
  TARGETS=(board)
  BUILD_SYSTEM="arduino-cli"
  FQBN="arduino:avr:uno"
  ARDUINO_CORE="arduino:avr"
  ARDUINO_BOARD_URLS=""
  SIMAVR_MCU="atmega328p"
  SIMAVR_FREQUENCY=16000000
  DATABASE_EXAMPLE="Basics"
  SELF_CHECK_EXAMPLE="SelfCheck"
  VARIANTS=(default)
  TIDY_VARIANTS=(default)
  DOCS_INTERNAL_API=""
  CPPCHECK=1
  HOST_CODE=()
}

# The hooks, which do nothing unless tools/project.sh defines them again, and
# then are never called: SC2329.
# shellcheck disable=SC2329
{
  variant_flags() {
    :
  }

  prepare() {
    :
  }

  target_settings() {
    :
  }

  check_elf() {
    :
  }
}

# shellcheck source=tools/project.sh
source "${ROOT_DIR}/tools/project.sh"

# The settings a target may have of its own, through target_settings() in tools/project.sh; what
# tools/project.sh sets for them is each target's starting point. Kept as declarations, with -g,
# so that select_target() restores them as globals although it is a function.
readonly TARGET_SETTINGS=(BUILD_SYSTEM FQBN ARDUINO_CORE ARDUINO_BOARD_URLS SIMAVR_MCU
  SIMAVR_FREQUENCY DATABASE_EXAMPLE SELF_CHECK_EXAMPLE)
PROJECT_SETTINGS="$(declare -p "${TARGET_SETTINGS[@]}" | sed 's/^declare /declare -g /')"
readonly PROJECT_SETTINGS

# shellcheck source=tools/lib/arduino_cli.sh
source "${ROOT_DIR}/tools/lib/arduino_cli.sh"

err() {
  echo "$*" >&2
}

# Makes the target named the one the tools work on: TARGET is its name, and the settings of
# TARGET_SETTINGS are those of tools/project.sh with what target_settings() sets for it. Fails on
# a target TARGETS does not name or whose name is no folder's, and on a build system the template
# does not know, as a misspelt one would build nothing without a word.
# Arguments:
#   target: one of TARGETS
select_target() {
  local target="$1"
  if [[ " ${TARGETS[*]} " != *" ${target} "* ]]; then
    err "${target} is no target of TARGETS in tools/project.sh: ${TARGETS[*]}."
    return 1
  fi
  if [[ ! "${target}" =~ ^[a-z][a-z0-9-]*$ ]]; then
    err "Target ${target}: a name is lower case, digits and -, as it names folders of build/."
    return 1
  fi
  eval "${PROJECT_SETTINGS}"
  TARGET="${target}"
  target_settings "${target}"
  case "${BUILD_SYSTEM}" in
    arduino-cli) ;;
    *)
      err "Target ${target}: BUILD_SYSTEM=${BUILD_SYSTEM}, which the template does not know;" \
        "it knows arduino-cli."
      return 1
      ;;
  esac
}

# Prints the folder the builds of the selected target go into, one folder per variant below it.
target_build_dir() {
  echo "${BUILD_DIR}/targets/${TARGET}"
}

# Prints what names a variant of the selected target in a message or a table: the variant alone
# where the project has one target, so that its table of sizes reads as it always has, else
# target/variant.
# Arguments:
#   variant: one of VARIANTS
target_label() {
  local variant="$1"
  if ((${#TARGETS[@]} == 1)); then
    echo "${variant}"
  else
    echo "${TARGET}/${variant}"
  fi
}

# Fills the array named first with the files git has below the pathspecs whose paths match the
# regex: tracked ones and new ones not yet added, but not what .gitignore names, so that a file
# is checked before its first commit. Fails where git fails or no file matches, as a check over
# no file would pass.
# Arguments:
#   array: the name of the array to fill
#   regex: an extended regular expression, such as '\.(cpp|h)$'
#   pathspec...: what git lists, such as 'src/*'
list_files() {
  local -n list_files_into="$1"
  local regex="$2"
  shift 2
  local listed
  listed="$(git ls-files --cached --others --exclude-standard -- "$@")"
  if ! listed="$(grep -E "${regex}" <<< "${listed}")"; then
    err "No file in $* matches ${regex}."
    return 1
  fi
  # shellcheck disable=SC2034  # a nameref: what it fills is the caller's array
  mapfile -t list_files_into <<< "${listed}"
}

# Fills the array named first with the folders of HOST_CODE, each without the / at its end. Fails
# where one is no folder, as a misspelt one would be checked by nothing, without a word.
# Arguments:
#   array: the name of the array to fill
host_code_dirs() {
  local -n host_code_dirs_into="$1"
  host_code_dirs_into=()
  local dir
  for dir in "${HOST_CODE[@]}"; do
    dir="${dir%/}"
    if [[ ! -d "${ROOT_DIR}/${dir}" ]]; then
      err "HOST_CODE in tools/project.sh names ${dir}, which is no folder of the project."
      return 1
    fi
    host_code_dirs_into+=("${dir}")
  done
}

# The first target, selected for a script that never selects one, as a script of the project's
# own written before there were targets: it finds FQBN and the rest set as they always were.
select_target "${TARGETS[0]}"
