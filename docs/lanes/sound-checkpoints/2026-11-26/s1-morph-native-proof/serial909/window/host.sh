#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")";pwd); r=$(dirname "$d")
ref="$r/ui-native-table-share/tree/firmware/src/dsp"
src="$d/tree/firmware/src/dsp"; gen="$d/tree/build/smk37-elite/gen"
out="$d/host"; mkdir -p "$out"
flag=(-Os ${HOST_SANITIZE:-} -ffp-contract=off -ffunction-sections -fdata-sections -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1 -I"$src" -I"$gen")
cc "${flag[@]}" -c "$ref/drum909.c" -o "$out/original.o"
nm -g --defined-only "$out/original.o" | awk 'NF==3 {print $3,"ref_"$3}' >"$out/symbols"
objcopy --redefine-syms="$out/symbols" "$out/original.o" "$out/ref.o"
cc "${flag[@]}" "$d/identity.c" "$src/drum909.c" "$out/ref.o" "$src/x0x_param.c" -Wl,--gc-sections -lm -o "$out/test"
"$out/test"
