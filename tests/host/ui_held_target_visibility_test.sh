#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/ui-held-target-visibility} \
SMC_UI_TEST_FILE=tests/host/ui_held_target_visibility_test.c sh tests/host/smc_ui_test.sh
