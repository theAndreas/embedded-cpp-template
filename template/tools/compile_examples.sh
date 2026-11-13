#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Compiles every example for each target in each variant and prints flash and RAM of each, as a
# Markdown table: a column per variant, named target/variant where there is more than one target.
# Fails on a warning outside the Arduino core, and where check_elf() of tools/project.sh fails
# on what was built. When every example compiles, the table goes to build/sizes.md as well, which
# tools/documented_sizes.py compares with the README's (`make sizes`); else that file is removed,
# so that no table of an older build is taken for this one's.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

# Prints the number after "<label> use(s)" in the output of arduino-cli.
size_of() {
  local label="$1"
  local output="$2"
  sed -n "s/^${label} uses* \([0-9]*\) bytes.*/\1/p" <<< "${output}"
}

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi

  local failed=0
  local target
  local example
  local variant
  local label
  local output
  local warnings
  local elf
  local -A flash_size
  local -A ram_size
  local labels=()
  local listed
  listed="$(list_examples)"
  local examples
  mapfile -t examples <<< "${listed}"
  local -r sizes="${BUILD_DIR}/sizes.md"
  rm -f "${sizes}"

  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    for variant in "${VARIANTS[@]}"; do
      labels+=("$(target_label "${variant}")")
    done
    for example in "${examples[@]}"; do
      for variant in "${VARIANTS[@]}"; do
        label="$(target_label "${variant}")"
        if ! output="$(compile_example "${variant}" "${example}")"; then
          err "${example} (${label}) does not compile:"
          err "${output}"
          failed=1
          continue
        fi
        warnings="$(grep 'warning:' <<< "${output}" \
          | grep -v '/cores/arduino/' || true)"
        if [[ -n "${warnings}" ]]; then
          err "${example} (${label}) warns:"
          err "${warnings}"
          failed=1
        fi
        elf="$(elf_of "${variant}" "${example}")"
        check_elf "${variant}" "${elf}" || failed=1
        flash_size["${example}/${label}"]="$(size_of 'Sketch' "${output}")"
        ram_size["${example}/${label}"]="$(size_of 'Global variables' "${output}")"
      done
    done
  done

  local header="| Example |"
  local rule="|---|"
  for label in "${labels[@]}"; do
    header+=" Flash, ${label} |"
    rule+="---|"
  done
  for label in "${labels[@]}"; do
    header+=" RAM, ${label} |"
    rule+="---|"
  done
  local table="${header}"$'\n'"${rule}"
  local row
  for example in "${examples[@]}"; do
    row="| ${example} |"
    for label in "${labels[@]}"; do
      row+=" ${flash_size["${example}/${label}"]:-} |"
    done
    for label in "${labels[@]}"; do
      row+=" ${ram_size["${example}/${label}"]:-} |"
    done
    table+=$'\n'"${row}"
  done
  echo "${table}"
  if ((failed == 0)); then
    mkdir -p "${BUILD_DIR}"
    echo "${table}" > "${sizes}"
  fi
  exit "${failed}"
}

main "$@"
