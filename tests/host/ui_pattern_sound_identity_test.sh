#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
export SMC_UI_TEST_FILE=tests/host/ui_pattern_sound_identity_test.c
export SMC_UI_OUT=${SMC_UI_OUT:-build/host/pattern-sound-identity}
exec sh tests/host/smc_ui_test.sh
