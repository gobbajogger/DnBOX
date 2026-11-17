#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_drum_single_lane_test.c
export SMC_UI_OUT=${SMC_UI_OUT:-build/host/drum-single-lane}
sh tests/host/smc_ui_test.sh
