#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
X0X_HOST_DEFS="${X0X_HOST_DEFS:-} -DX0X_DNR2_PROBE=1" \
SMC_UI_OUT=${SMC_UI_OUT:-build/host/remote-events} \
SMC_UI_TEST_FILE=tests/host/remote_events_test.c sh tests/host/smc_ui_test.sh
