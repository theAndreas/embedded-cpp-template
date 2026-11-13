#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The build system arduino-cli: how a target with BUILD_SYSTEM=arduino-cli is set up, how an
# example is compiled for it, and the flags clang-tidy reads it with. tools/common.sh sources this
# file; every function works on the target select_target() selected, through its FQBN,
# ARDUINO_CORE, ARDUINO_BOARD_URLS and DATABASE_EXAMPLE. This file comes from the template.

# Prints the name of every example, one per line.
list_examples() {
  local dir
  for dir in "${ROOT_DIR}"/examples/*/; do
    basename "${dir}"
  done
}

# Installs the core of the selected target, pinned, from the index of ARDUINO_BOARD_URLS as well
# if it names one.
arduino_setup() {
  local urls=()
  if [[ -n "${ARDUINO_BOARD_URLS}" ]]; then
    urls=(--additional-urls "${ARDUINO_BOARD_URLS}")
  fi
  arduino-cli core update-index "${urls[@]}"
  arduino-cli core install "${ARDUINO_CORE}" "${urls[@]}"
}

# Prints the bin directory of the avr-gcc that arduino-cli uses for FQBN.
avr_gcc_bin_dir() {
  local properties
  properties="$(arduino-cli compile --fqbn "${FQBN}" --show-properties \
    "${ROOT_DIR}/examples/${DATABASE_EXAMPLE}" 2>/dev/null)"
  sed -n 's/^compiler\.path=//p' <<< "${properties}"
}

# Compiles an example in a variant into <target build dir>/<variant>/<example> and prints what
# arduino-cli prints. The flags of the variant go into compiler.cpp.extra_flags, which reaches
# every C++ file of the build, the core's as well. Its callers test its status, which turns off
# set -e within it, so it returns on a failure itself.
# Arguments:
#   variant: one of VARIANTS
#   example: the name of a folder in examples/
compile_example() {
  local variant="$1"
  local example="$2"
  local args=(
    --fqbn "${FQBN}"
    --library "${ROOT_DIR}"
    --warnings all
    --build-path "$(target_build_dir)/${variant}/${example}"
  )
  local flags
  flags="$(variant_flags "${variant}")" || return
  flags="${flags//$'\n'/ }"
  if [[ -n "${flags}" ]]; then
    args+=(--build-property "compiler.cpp.extra_flags=${flags}")
  fi
  arduino-cli compile "${args[@]}" "${ROOT_DIR}/examples/${example}" 2>&1
}

# Prints the path of the ELF file that compile_example made.
elf_of() {
  local variant="$1"
  local example="$2"
  local dir
  dir="$(target_build_dir)"
  echo "${dir}/${variant}/${example}/${example}.ino.elf"
}

# Prints, as a JSON array, the arguments arduino-cli compiles a file of the sketch
# DATABASE_EXAMPLE with in VARIANT, without the file, its output and -w, and changed for clang.
# Compiled with the defines of the variant, as arduino-cli finds the libraries a sketch needs by
# its includes: one that only a variant includes, as SPI under a switch, has its folder in the
# database of that variant alone. Only the defines: a plugin or an optimization of avr-gcc means
# nothing to clang.
# - The include folders outside the repository, the core's and the board's, become -isystem,
#   so that nothing is reported from headers we do not own.
# - -Wall -Wextra, so that clang's own warnings are reported as well.
# - -Wno-unknown-attributes: clang does not know avr-gcc's attribute __progmem__, which
#   PROGMEM stands for, and takes the tables as in RAM. That changes nothing clang-tidy checks.
# Arguments:
#   variant: one of TIDY_VARIANTS
#   dir: a folder for arduino-cli's own build and database
arduino_flags() {
  local variant="$1"
  local dir="$2"
  local flags
  flags="$(variant_flags "${variant}")"
  local flag
  local defines=()
  while IFS= read -r flag; do
    if [[ "${flag}" == -D* ]]; then
      defines+=("${flag}")
    fi
  done <<< "${flags}"
  local args=(
    --fqbn "${FQBN}"
    --library "${ROOT_DIR}"
    --only-compilation-database
    --build-path "${dir}"
  )
  if ((${#defines[@]} > 0)); then
    args+=(--build-property "compiler.cpp.extra_flags=${defines[*]}")
  fi
  arduino-cli compile "${args[@]}" "${ROOT_DIR}/examples/${DATABASE_EXAMPLE}" >/dev/null
  jq --arg root "${ROOT_DIR}" '[.[] | select(.file | endswith(".ino.cpp"))][0]
    | .file as $file
    | .arguments
    | del(.[index("-o"), index("-o") + 1])
    | map(select(. != $file and . != "-w"))
    | map(if startswith("-I") and (startswith("-I" + $root) | not)
          then "-isystem" + .[2:] else . end)
    + ["-Wall", "-Wextra", "-Wno-unknown-attributes"]' \
    "${dir}/compile_commands.json"
}
