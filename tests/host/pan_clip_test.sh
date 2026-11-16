#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The pan clip at 50: every entry point, the law, knob travel, load and readout/marker.
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/pan_clip_test.c
export SMC_UI_OUT="${PAN_CLIP_OUT:-build/host/pan-clip}"
exec sh tests/host/smc_ui_test.sh
