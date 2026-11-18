#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/break-outline-snapshot} \
SMC_UI_TEST_FILE=tests/host/break_outline_snapshot_test.c sh tests/host/smc_ui_test.sh
