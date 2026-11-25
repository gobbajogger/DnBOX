#!/bin/sh
set -eu
if [ "$#" -ne 2 ]; then echo "usage: $0 approved-source-root output.so" >&2; exit 2; fi
here="$(cd "$(dirname "$0")" && pwd)"
tree="$(realpath "$1")"
src="$tree/firmware/src"
out="$2"
[ -f "$src/dsp/juno6_j60_native.inc" ] && [ -f "$src/dsp/juno6_delay_native.inc" ] && [ -f "$src/dsp/sj6_native_room.inc" ]
exec "${CC:-cc}" -shared -fPIC -std=c11 -O2 -ffp-contract=off -fno-fast-math \
    -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 \
    -DX0X_SJ6_NATIVE_J60=1 -DX0X_SJ6_NATIVE_CLASSIC=1 -DX0X_SJ6_NATIVE_DELAY=1 \
    -DX0X_SJ6_NATIVE_ROOM=1 -I"$src" -I"$tree/experiments/original_synth" "$here/compact_bridge.c" -lm -o "$out"
