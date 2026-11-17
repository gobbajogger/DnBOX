#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_drum_shared_glyph_test.c
export SMC_UI_OUT=${1:-build/host/ui-drum-shared-glyph}
exec sh tests/host/smc_ui_test.sh
