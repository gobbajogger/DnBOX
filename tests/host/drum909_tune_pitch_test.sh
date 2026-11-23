#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# 909 TUNE vs PITCH (FINAL-GOAL row 16) against the last pre-change drum909.c (on
# integration/dnbox-consolidated): defaults bit-identical, the two controls independent.
set -eu
cd "$(dirname "$0")/../.."
BASE=${BASE:-76040daa5b52d686a8b7b544296ba5d79610a95d}
OUT=build/host/drum909-tune-pitch; mkdir -p "$OUT"
: "${CC:=cc}"
[ -f build/gen/x0x_drum_tables.h ] || python3 tools/gen_drum_samples.py build/gen/x0x_drum_samples.h >/dev/null
git show "$BASE:firmware/src/dsp/drum909.c" > "$OUT/base.c"
flags="-std=c99 -O2 -ffp-contract=off -fno-fast-math -DX0X_HOST -Wall -Wextra -Werror -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen"
$CC $flags -c "$OUT/base.c" -o "$OUT/base.o"
nm -g --defined-only "$OUT/base.o" | awk '{print $3 " ref_" $3}' > "$OUT/symbols"
objcopy --redefine-syms="$OUT/symbols" "$OUT/base.o" "$OUT/ref.o"
$CC $flags -c firmware/src/dsp/drum909.c -o "$OUT/candidate.o"
$CC $flags tests/host/drum909_tune_pitch_test.c "$OUT/ref.o" "$OUT/candidate.o" firmware/src/dsp/x0x_param.c -lm -o "$OUT/test"
"$OUT/test"
