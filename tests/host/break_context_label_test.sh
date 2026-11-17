#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_TEST_FILE=tests/host/break_context_label_test.c SMC_UI_OUT=${SMC_UI_OUT:-build/host/break-context-label} sh tests/host/smc_ui_test.sh
