#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Compare the compact provider before and after extracting shared physical units.
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/juno-audio-replay}
mkdir -p "$out"
base=3e67b59c8e4dd3d5994680d857ec4e2ea755ab6d
. tests/host/original_build.sh
sources=$(python3 -c "import tools.build as b; print(' '.join(p for p in b.ORIGINAL_SYNTH_UNITS+b.ORIGINAL_SYNTH_BACKENDS if not p.endswith(('/original_runtime.c','/juno6.c'))))")
git show "$base:firmware/src/app/original_runtime.c" > "$out/base-runtime.c"
git show "$base:firmware/src/dsp/juno6.c" > "$out/base-juno.c"
for side in before after; do
    runtime=firmware/src/app/original_runtime.c
    synth=firmware/src/dsp/juno6.c
    if [ "$side" = before ]; then
        runtime="$out/base-runtime.c"
        synth="$out/base-juno.c"
    fi
    ${CC:-cc} -std=c99 -O2 -ffunction-sections -fdata-sections -DX0X_HOST $ORIGINAL_FLAGS \
        -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
        tests/host/juno_slot_audio_replay.c "$runtime" "$synth" $sources -Wl,--gc-sections -lm -o "$out/$side"
    "$out/$side" > "$out/$side.f32"
done
cmp "$out/before.f32" "$out/after.f32"
sha256sum "$out/before.f32" "$out/after.f32"
