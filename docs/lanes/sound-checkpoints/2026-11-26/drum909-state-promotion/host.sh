#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")";pwd); r=$(dirname "$d")
T="/tmp/phaser-warm-own.Y0XNbQ/tree"; s="$T/firmware/src"; gen="$T/build/smk37-elite/gen"
o=$(mktemp -d $d/host-out.XXXX);trap 'rm -r "$o"' EXIT
f=(-Os -ffp-contract=off -ffunction-sections -fdata-sections -DX0X_909_HUFF_DIAG=${HUFF:-1} -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$s/dsp" -I"$s" -I"$gen")
cc "${f[@]}" -c "$d/base.c" -o "$o/base.o"
nm -g --defined-only "$o/base.o" | awk 'NF==3 {print $3,"ref_"$3}' >"$o/renames"
objcopy --redefine-syms="$o/renames" "$o/base.o" "$o/ref.o"
cc "${f[@]}" "$d/host.c" "$d/candidate.c" "$o/ref.o" "$s/dsp/x0x_param.c" -Wl,--gc-sections -lm -o "$o/run"
"$o/run"
