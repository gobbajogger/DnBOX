#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Pan display L50..C..R50 over raw 0..127: no clamp, display-step edits, byte-identical legacy load, readout/marker.
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/pan_clip_test.c
export SMC_UI_OUT="${PAN_CLIP_OUT:-build/host/pan-clip}"
exec sh tests/host/smc_ui_test.sh
