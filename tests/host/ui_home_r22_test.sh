#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_home_r22_test.c
export SMC_UI_OUT=${1:-build/host/ui-home-r22}
exec sh tests/host/smc_ui_test.sh
