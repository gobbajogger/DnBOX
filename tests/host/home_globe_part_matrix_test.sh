#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# HOME GLOBE+pad clear/copy touches only the selected part, for every part.
# Cold trees lack build/gen; generate first (same gate as song_actions).
set -e
cd "$(dirname "$0")/../.."
. tests/host/original_build.sh
OUT=${OUT:-build/host}
mkdir -p "$OUT" build/gen
sh host/build_host.sh --generate-only >/dev/null
${CC:-cc} -O2 -ffp-contract=off -std=c99 -w -DX0X_HOST $ORIGINAL_FLAGS -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/home_globe_part_matrix_test.c $ORIGINAL_SOURCES -lm -o "$OUT/home_globe_part_matrix_test"
"$OUT/home_globe_part_matrix_test"
