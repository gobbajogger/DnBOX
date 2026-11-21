#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/drum808-kit-panel}
mkdir -p "$out"
sh host/build_host.sh --generate-only >"$out/assets.log" 2>&1
. tests/host/original_build.sh
${CC:-cc} -std=c11 -O1 -DX0X_HOST -DX0X_SYNTHONE_PATCH=0 -D_POSIX_C_SOURCE=200809L \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    $ORIGINAL_FLAGS ${SAN_FLAGS:-} tests/host/drum808_kit_panel_test.c $ORIGINAL_SOURCES -lm -o "$out/test"
for kit in 0 1; do
    "$out/test" "$kit" 0 0 "$out/$kit-dry.pcm"
    "$out/test" "$kit" 0 1 "$out/$kit-restored-dry.pcm"
    for mode in 1 2 3 4 5 6 7 8; do
        "$out/test" "$kit" "$mode" 0 "$out/$kit-edit.pcm"
        if [ "$kit" = 0 ]; then
            cmp "$out/$kit-dry.pcm" "$out/$kit-edit.pcm"
        elif cmp -s "$out/$kit-dry.pcm" "$out/$kit-edit.pcm"; then
            echo "808 control $mode left the audio unchanged" >&2
            exit 1
        fi
        "$out/test" "$kit" "$mode" 1 "$out/$kit-restore.pcm"
        "$out/test" "$kit" "$mode" 2 "$out/$kit-expected.pcm"
        cmp "$out/$kit-expected.pcm" "$out/$kit-restore.pcm"
        if [ "$kit" = 0 ]; then
            cmp "$out/$kit-restored-dry.pcm" "$out/$kit-restore.pcm"
        elif cmp -s "$out/$kit-restored-dry.pcm" "$out/$kit-restore.pcm"; then
            echo "808 control $mode did not survive project reload" >&2
            exit 1
        fi
    done
done
echo '808 KIT: every shape and glue changed the 808, left the 909 unchanged, and survived project reload sample-for-sample.'
