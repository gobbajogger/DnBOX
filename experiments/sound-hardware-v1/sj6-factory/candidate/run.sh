#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
here=$(cd "$(dirname "$0")" && pwd)
tree=$(cd "${S4_FIRMWARE_TREE:-$here/../../../..}" && pwd)
out=$1
shift
mkdir "$out"
out=$(cd "$out" && pwd)
cd "$tree"
export X0X_SUPERJ6_FACTORY=1
export X0X_HOST_DEFS="-Itests/host -Wl,--wrap=juno6_init"
export SMC_UI_TEST_FILE="$here/probe.c"
export SMC_UI_OUT="$out/build"
sh tests/host/smc_ui_test.sh > "$out/build.log" 2>&1 || { cat "$out/build.log"; exit 1; }
python3 "$here/diagnose.py" "$tree" "$out" "$@"
