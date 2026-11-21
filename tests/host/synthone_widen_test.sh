#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/synthone-widen}
mkdir -p "$out"
out=$(cd "$out" && pwd)
sh host/build_host.sh --generate-only >"$out/assets.log" 2>&1
if [ -z "${S1_ORIGINAL_INPUTS:-}" ]; then
    S1_ORIGINAL_INPUTS="$out/inputs"
    python3 tools/restore-synthone-test-inputs.py --bundle assets/test-inputs/synthone --output "$S1_ORIGINAL_INPUTS"
fi
sp="$S1_ORIGINAL_INPUTS/closure/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe"
. tests/host/original_build.sh
${CC:-cc} -std=c11 -O1 -DX0X_HOST -DX0X_SYNTHONE_PATCH=1 -DX0X_SYNTHONE_WIDEN=1 \
    -DX0X_WIDEN_PROBE -D_POSIX_C_SOURCE=200809L -Ifirmware/src -Ifirmware/src/app \
    -Ifirmware/src/dsp -Ibuild/gen -I"$sp" -I"$sp/lib/dr_wav" $ORIGINAL_FLAGS ${SAN_FLAGS:-} \
    tests/host/synthone_patch_test.c "$sp/modules/port.c" $ORIGINAL_SOURCES -lm -o "$out/test"
"$out/test"
