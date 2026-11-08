#!/bin/sh
#
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
#
# The editor metadata in devcontainer-feature.json is the whole of this feature;
# the tools are installed by the template's Dockerfile, and the Arduino core by the `make setup`
# of a project that tests/check.sh makes, as it lives in a volume rather than in the image.

set -e

echo "embedded-cpp-template shared configuration: nothing to install."
