#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# SEQ pads write 909/808/BREAK steps and their LEDs follow the steps and the playhead.
# Cold trees lack build/gen; generate first (same gate as song_actions).
set -e
cd "$(dirname "$0")/../.."
. tests/host/original_build.sh
OUT=${OUT:-build/host}
mkdir -p "$OUT" build/gen
sh host/build_host.sh --generate-only >/dev/null
${CC:-cc} -O2 -ffp-contract=off -std=c99 -w -DX0X_HOST $ORIGINAL_FLAGS -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/seq_pad_led_test.c $ORIGINAL_SOURCES -lm -o "$OUT/seq_pad_led_test"
"$OUT/seq_pad_led_test"
