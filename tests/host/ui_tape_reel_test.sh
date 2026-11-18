#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/tape-reel} SMC_UI_TEST_FILE=tests/host/ui_tape_reel_test.c sh tests/host/smc_ui_test.sh
