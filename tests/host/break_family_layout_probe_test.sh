#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/break-family-layout-probe} \
SMC_UI_TEST_FILE=tests/host/break_family_layout_probe_test.c sh tests/host/smc_ui_test.sh
