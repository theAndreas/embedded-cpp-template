#!/bin/sh
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The editor metadata in devcontainer-feature.json, and the Ubuntu packages of the repository's
# own: the tools every project needs are installed by the template's Dockerfile, and the Arduino
# core by the `make setup` of a project that tests/check.sh makes, as it lives in a volume rather
# than in the image. What a project names in its .devcontainer/apt-packages.txt, the repository
# has only as a template; so it installs here what tests/check.sh needs of it: gcc-avr and
# avr-libc, which a library of CMake alone and a firmware build their targets with. The
# template's CI installs the same (.github/workflows/template.yml).

set -e

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends gcc-avr avr-libc
rm -rf /var/lib/apt/lists/*
