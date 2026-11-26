#!/bin/bash
set -euo pipefail
D=$(realpath "$(dirname "$0")"); donor=$(realpath "$1")
w=$(mktemp -d /tmp/dnbox-serial-proof.XXXXX);trap 'rm -r "$w"' EXIT
s="$donor/firmware/src"
F=(-Os -ffp-contract=off -ffunction-sections -fdata-sections -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_TANH_SPLIT_LAB=1 -I"$s/dsp" -I"$s" -I"$donor/build/smk37-elite/gen")
cp "$s/dsp/drum909.c" "$w/drum.c"; python "$D/strip-compare.py" "$w/drum.c"
sed -i 's|"../x0x_cold.h"|"x0x_cold.h"|' "$w/drum.c"
cc "${F[@]}" -c "$w/drum.c" -o "$w/strip.o"
nm -g --defined-only "$w/strip.o" | awk 'NF==3 {print $3,"stripped_"$3}' >"$w/renames"
objcopy --redefine-syms="$w/renames" "$w/strip.o" "$w/test.o"
cc "${F[@]}" "$D/host.c" "$s/dsp/"{drum909,x0x_param}.c "$w/test.o" -Wl,--gc-sections -lm -o "$w/verify"
"$w/verify"
