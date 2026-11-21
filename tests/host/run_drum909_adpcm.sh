#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# The 909 cymbals as IMA ADPCM: tests/host/drum909_adpcm_test.c against tests/host/cymbal_ref.py's
# references (the approved A/B study in ../adpcm_ab when it is there; set CYM_AB to another folder).
#   tests/host/run_drum909_adpcm.sh
set -e
cd "$(dirname "$0")/../.."
CC="${CC:-cc}"
OUT=build/host
mkdir -p "$OUT"
python3 tools/gen_drum_samples.py build/gen/x0x_drum_samples.h >/dev/null
for mode in production historical; do
    ref="$OUT/cym_ref/$mode"
    if [ "$mode" = historical ]; then
        python3 tests/host/cymbal_ref.py --historical "$ref" ${CYM_AB:+"$CYM_AB"}
    else
        python3 tests/host/cymbal_ref.py "$ref"
    fi
    # The dated arm compiles the same firmware decoder against the pinned old streams.
    "$CC" -std=c99 -O2 -ffp-contract=off -Wall -Wextra -Wdouble-promotion -Wno-unused-function -Werror \
        -Ifirmware/src -Ifirmware/src/dsp -I"$ref" -Ibuild/gen -o "$OUT/drum909_adpcm_$mode" tests/host/drum909_adpcm_test.c -lm
    "$OUT/drum909_adpcm_$mode" "$ref"
done
