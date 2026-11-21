#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/synthone-crush-panel}
mkdir -p "$out"
sh host/build_host.sh --generate-only >"$out/assets.log" 2>&1
. tests/host/original_build.sh
${CC:-cc} -std=c11 -O1 -DX0X_HOST -DX0X_SYNTHONE_PATCH=0 -D_POSIX_C_SOURCE=200809L \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    $ORIGINAL_FLAGS ${SAN_FLAGS:-} tests/host/synthone_crush_panel_test.c $ORIGINAL_SOURCES -lm -o "$out/test"
"$out/test"
