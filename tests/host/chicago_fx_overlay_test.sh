#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
OUT=${CHICAGO_OVERLAY_OUT:-build/host/chicago-fx-overlay}
mkdir -p "$OUT"
sh host/build_host.sh --generate-only > "$OUT/generate.log" 2>&1
. tests/host/original_build.sh
SOURCES=
for source in $ORIGINAL_SOURCES; do
 case "$source" in firmware/src/app/engine.c) ;; *) SOURCES="$SOURCES $source" ;; esac
done
${CC:-cc} -O1 -std=c99 -ffunction-sections -fdata-sections -Wl,--gc-sections \
 -Ibuild/gen -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp \
 $ORIGINAL_FLAGS ${SAN_FLAGS:-} tests/host/chicago_fx_overlay_test.c $SOURCES -lm -o "$OUT/test"
"$OUT/test"
