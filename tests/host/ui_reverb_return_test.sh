#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_TEST_FILE=tests/host/ui_reverb_return_test.c exec sh tests/host/smc_ui_test.sh
