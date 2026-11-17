#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_shared_parameter_panel_test.c
export SMC_UI_OUT=${SMC_UI_OUT:-build/host/shared-parameter-panel}
sh tests/host/smc_ui_test.sh
