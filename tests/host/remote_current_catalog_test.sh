#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/remote-current-catalog}
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)
DNBOX_CURRENT_CATALOG_OUT="$OUT/census.json" SMC_UI_OUT="$OUT" SMC_UI_TEST_FILE=tests/host/remote_current_catalog_test.c sh tests/host/smc_ui_test.sh
