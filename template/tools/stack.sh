#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Checks that the deepest stack of every program, the examples of a library or the image of a
# firmware, fits the RAM it leaves free, on every AVR target that CMake builds, in each variant:
# tools/avr_stack.py adds it up from the call graphs avr-gcc writes beside the objects. Builds
# first, as make examples does. Leaves out a target built by arduino-cli: arduino:avr's avr-gcc is
# 7.3, and -fcallgraph-info came with GCC 10. Also leaves out a target CMake builds for another
# processor; none is there yet.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi

  local failed=0
  local checked=0
  local target
  local variant
  local label
  local listed
  local programs
  local program
  local mcu
  local elf
  local graphs
  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    if [[ "${BUILD_SYSTEM}" != cmake ]]; then
      echo "stack.sh: ${target} is built by ${BUILD_SYSTEM}, whose avr-gcc writes no call graph."
      continue
    fi
    for variant in "${VARIANTS[@]}"; do
      label="$(target_label "${variant}")"
      if ! target_build "${variant}"; then
        failed=1
        continue
      fi
      # The part, which cmake/toolchains/avr-gcc.cmake keeps in the cache; a toolchain file for
      # another processor has none.
      mcu="$(cmake_cache "${variant}" AVR_MCU)"
      if [[ -z "${mcu}" ]]; then
        echo "stack.sh: ${target} is no AVR, and only an AVR's stack is added up."
        break
      fi
      listed="$(target_programs "${variant}")"
      mapfile -t programs <<< "${listed}"
      for program in "${programs[@]}"; do
        checked=1
        elf="$(target_elf "${variant}" "${program}")"
        graphs="$(cmake_graphs "${variant}" "${program}")"
        echo "${program} (${label}):"
        python3 "${ROOT_DIR}/tools/avr_stack.py" --mcu "${mcu}" "${elf}" "${graphs}" || failed=1
      done
    done
  done
  if ((!checked)); then
    echo "stack.sh: no AVR target built by CMake, nothing to add up."
  fi
  exit "${failed}"
}

main "$@"
