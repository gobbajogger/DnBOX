#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
out=${1:-build/host/drum808-reverb-panel}
mkdir -p "$out"
sh host/build_host.sh --generate-only >"$out/assets.log" 2>&1
. tests/host/original_build.sh
${CC:-cc} -std=c11 -O1 -DX0X_HOST -DX0X_SYNTHONE_PATCH=0 -D_POSIX_C_SOURCE=200809L \
    -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
    $ORIGINAL_FLAGS ${SAN_FLAGS:-} tests/host/drum808_reverb_panel_test.c $ORIGINAL_SOURCES -lm -o "$out/test"
for kit in 0 1; do
    "$out/test" "$kit" 0 0 "$out/base"
    "$out/test" "$kit" 0 1 "$out/legacy"
    "$out/test" "$kit" 0 2 "$out/shared"
    cmp "$out/shared" "$out/legacy"
    for mode in 1 2 3 4 5; do
        "$out/test" "$kit" "$mode" 0 "$out/mode-$mode"
        "$out/test" "$kit" "$mode" 1 "$out/restored"
        "$out/test" "$kit" "$mode" 2 "$out/expected"
        cmp "$out/expected" "$out/restored"
        if [ "$kit" = 0 ]; then
            cmp "$out/legacy" "$out/restored"
        elif cmp -s "$out/legacy" "$out/restored"; then
            echo "Reloaded 808 private reverb $mode is dry" >&2; exit 1
        fi
        if [ "$kit" = 0 ]; then
            cmp "$out/base" "$out/mode-$mode"
        elif cmp -s "$out/base" "$out/mode-$mode"; then
            echo "808 private reverb mode $mode left the audio unchanged" >&2; exit 1
        elif [ "$mode" != 1 ] && cmp -s "$out/mode-1" "$out/mode-$mode"; then
            echo "808 private reverb control $mode had no effect" >&2; exit 1
        fi
    done
done
echo '808 reverb route, four live panel knobs, legacy shared project, full restore, isolated 909 PCM and wet 808 audio passed.'
