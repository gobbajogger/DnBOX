#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Actual R22 909/808 renderer with pixel-weight and complete-voice checks.
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/ui-r22-drum} SMC_UI_TEST_FILE=tests/host/ui_r22_drum_test.c sh tests/host/smc_ui_test.sh
