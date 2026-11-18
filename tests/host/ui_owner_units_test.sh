#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_owner_units_test.c
export SMC_UI_OUT=${1:-build/host/ui-owner-units}
exec sh tests/host/smc_ui_test.sh
