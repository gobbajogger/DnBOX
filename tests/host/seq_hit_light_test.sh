#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The SEQ grid lights a playhead step only on a real hit (rest, mute, chance 0 stay dark).
# Cold trees lack build/gen; generate first (same gate as song_actions).
set -e
cd "$(dirname "$0")/../.."
. tests/host/original_build.sh
OUT=${OUT:-build/host}
mkdir -p "$OUT" build/gen
sh host/build_host.sh --generate-only >/dev/null
${CC:-cc} -O2 -ffp-contract=off -std=c99 -w -DX0X_HOST $ORIGINAL_FLAGS -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/seq_hit_light_test.c $ORIGINAL_SOURCES -lm -o "$OUT/seq_hit_light_test"
"$OUT/seq_hit_light_test"
