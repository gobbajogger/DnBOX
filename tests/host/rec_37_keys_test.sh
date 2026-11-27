#!/bin/sh
# Run the actual HAL key FIFOs and production UI/engine, not a UI model.
set -eu
cd "$(dirname "$0")/../.."
X0X_LIVE_TEST_OUT=build/host/rec37 X0X_LIVE_CASES=tests/host/rec_37_keys_cases.inc \
X0X_LIVE_EXTRA_FLAGS=-DX0X_REC_KEY_TEST=1 X0X_REC_KEY_TEST=1 \
sh tests/host/freeplay_live_keys_test.sh
