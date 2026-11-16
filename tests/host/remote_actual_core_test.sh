#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/remote-actual-core}
X0X_HOST_DEFS="${X0X_HOST_DEFS:-} -DX0X_DNR2_PROBE=1" SMC_UI_OUT="$OUT" \
SMC_UI_TEST_FILE=tests/host/remote_actual_core_test.c sh tests/host/smc_ui_test.sh
case "$OUT" in /*) BIN="$OUT/test" ;; *) BIN="$PWD/$OUT/test" ;; esac
node tests/host/remote_actual_wire_test.mjs "$BIN"
"$BIN" --catalog > "$OUT/catalog.json"
