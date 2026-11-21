#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/synthone-init-audio}
mkdir -p "$out"
. tests/host/original_build.sh
backends=$(python3 -c 'import tools.build as b; print(" ".join(b.ORIGINAL_SYNTH_UNITS + b.ORIGINAL_SYNTH_BACKENDS))')
${CC:-cc} -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST $ORIGINAL_FLAGS \
    ${SAN_FLAGS:-} -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    tests/host/synthone_init_audio.c $backends -Wl,--gc-sections -lm -o "$out/test"
"$out/test"
