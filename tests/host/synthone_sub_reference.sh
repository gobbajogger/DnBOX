#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/synthone-sub-reference}
python3 tools/restore-synthone-test-inputs.py --bundle assets/test-inputs/synthone --output "$out/original"
source="$out/original/closure/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe"
${CC:-cc} -O2 -std=gnu11 -ffp-contract=off -fno-fast-math -DNO_LIBSNDFILE \
    -ffunction-sections -fdata-sections -Ifirmware/src/dsp -I"$source" -I"$source/lib/dr_wav" \
    tests/host/synthone_sub_reference.c "$source/modules/osc.c" "$source/modules/ftbl.c" \
    -Wl,--gc-sections -lm -o "$out/compare"
"$out/compare"
