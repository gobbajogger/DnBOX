#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${SMC_UI_OUT:-build/host/break-preview-visibility} \
SMC_UI_TEST_FILE=tests/host/ui_break_preview_visibility_test.c sh tests/host/smc_ui_test.sh
