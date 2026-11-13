#!/bin/bash
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Installs the Arduino core of each target of tools/project.sh, pinned, and builds what prepare()
# there builds beyond them. The dev container runs it when it is created, CI before it compiles an
# example; run it again after ARDUINO_CORE has changed.

set -euo pipefail

# shellcheck source=tools/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

main() {
  if [[ $# -gt 0 ]]; then
    err "Usage: $0"
    exit 2
  fi

  local target
  for target in "${TARGETS[@]}"; do
    select_target "${target}"
    arduino_setup
  done
  prepare
}

main "$@"
