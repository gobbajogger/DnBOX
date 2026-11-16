#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# All valid row/pot outputs must retain their original IEEE words.
set -eu
cd "$(dirname "$0")/.."
OUT=${EXP_CURVE_OUT:-build/host/exp-curve-exact}
mkdir -p "$OUT/generated"
python3 tools/gen_drum_samples.py "$OUT/generated/x0x_drum_samples.h"
for opt in O2 Os; do
    ${CC:-cc} -std=c99 -"$opt" -ffp-contract=off -fno-fast-math -Wall -Wextra -Werror \
        ${SAN_FLAGS:-} -I"$OUT/generated" tests/host/exp_curve_exact_test.c -o "$OUT/test-$opt"
    "$OUT/test-$opt"
done
