#!/bin/sh
# Synth One compact FX level vs original (X0X_S1C=1). Pack/fit table come from the box (not in Git).
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/s1c-fx-level}
mkdir -p "$out"
export X0X_S1C=1
. tests/host/original_build.sh
backends=$(X0X_S1C=1 python3 -c 'import tools.build as b; print(" ".join(b.ORIGINAL_SYNTH_UNITS + b.ORIGINAL_SYNTH_BACKENDS))')
${CC:-cc} -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=${X0X_SYNTHONE_PATCH:-1} $ORIGINAL_FLAGS \
    ${SAN_FLAGS:-} -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/s1c_fx_level_test.c $backends -Wl,--gc-sections -lm -o "$out/test"
"$out/test" ${S1_PACK:-}
