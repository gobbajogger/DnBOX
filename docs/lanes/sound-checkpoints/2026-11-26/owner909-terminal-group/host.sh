#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); T=${1:-/tmp/phaser-warm-own.Y0XNbQ/tree}
S="$T/firmware/src"; G="$T/build/smk37-elite/gen"
w=$(mktemp -d "$D/work.XXXX");trap 'rm -rf "$w"' EXIT
export TMPDIR="$w"
python3 "$D/group.py" "$S/dsp/drum909.c" >"$w/generated.c"
cat "$D/probe.inc" >>"$w/generated.c"
cc -std=gnu11 -O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -pthread -DX0X_HOST -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$S" -I"$S/dsp" -I"$G" "$w/generated.c" "$S/dsp/x0x_param.c" -Wl,--gc-sections -lm -o "$w/test"
"$w/test"
