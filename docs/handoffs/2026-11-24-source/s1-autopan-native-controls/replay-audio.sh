#!/bin/sh
set -eu
d=$(realpath "$(dirname "$0")"); root=$(realpath "$1"); src=$(realpath "$2")
w=$(mktemp -d "$root/.s1pan-host.XXXXXX")
trap 'rm -rf "$w"' EXIT HUP INT TERM
mkdir -p "$w/firmware/src/dsp"
cp -a "$src/firmware/src/dsp/fastmath.h" "$src/firmware/src/dsp/s1c" "$w/firmware/src/dsp/"
cd "$w"
patch --fuzz=0 -s -p1 < "$d/autopan-port.patch"
p=firmware/src/dsp/s1c
cc -std=c11 -O2 -ffp-contract=off -fsanitize=undefined -fno-sanitize-recover=all -DX0X_S1C=1 -DX0X_HOST=1 \
  -DX0X_S1_AUTOPAN_PROBE=1 -DX0X_S1_AUTOPAN_PORT=1 -I "$p" "$p/s1u.c" "$d/verify-audio.c" -lm -o "$w/test"
"$w/test"
