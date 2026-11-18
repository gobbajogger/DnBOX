#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/break-held-edit-journey} \
SMC_UI_TEST_FILE=tests/host/break_held_edit_journey_test.c sh tests/host/smc_ui_test.sh
