#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../../.."
out=${1:-build/host/synthone-master}
mkdir -p "$out"
. tests/host/original_build.sh
sp=firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe
sources=$(python3 -c 'import tools.build as b; print(" ".join(b.ORIGINAL_SYNTH_UNITS+b.ORIGINAL_SYNTH_BACKENDS))')
${CC:-cc} -std=c11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections \
    -DX0X_HOST -DX0X_SYNTHONE_PATCH=${X0X_SYNTHONE_PATCH:-0} -DNO_LIBSNDFILE -D_DEFAULT_SOURCE \
    $ORIGINAL_FLAGS -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Itests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe/lib/faust -I"$sp" -I"$sp/lib/dr_wav" \
    experiments/sound-hardware-v1/synthone-master/reference.c \
    tests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe/modules/compressor.c "$sp/modules/port.c" \
    $sources -Wl,--gc-sections -lm -o "$out/test"
if [ "${2:-}" = "--factory" ]; then
    python3 tools/compile-synthone-factory.py --out "$out/commands"
    fixture=tests/fixtures/synthone-factory
    ${CXX:-c++} -std=c++17 -O2 -I"$fixture/frontend" -I"$fixture/fullfx/compat" \
        experiments/sound-hardware-v1/synthone-autopan/factory-controls.cpp -o "$out/controls"
    "$out/controls" "$out/commands/preset-commands.bin" --master > "$out/targets"
    "$out/test" "$out/targets"
else
    "$out/test"
fi
