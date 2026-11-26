#!/bin/bash
set -euo pipefail
d="$(cd "$(dirname "$0")";pwd)"
src="$d/tree/firmware/src/dsp"; gen="$d/tree/build/smk37-elite/gen"
for mode in normal sanitize; do
 flags=(-Os -DX0X_909_HUFF_DIAG=1 -DX0X_COLD_COMPANION_LAB=1)
 if [[ "$mode" = sanitize ]];then flags+=(-g -fno-omit-frame-pointer -fsanitize=address,undefined -no-pie); fi
 cc "${flags[@]}" -ffunction-sections -fdata-sections -I"$src" -I"$gen" "$d/transaction-test.c" "$src/x0x_param.c" -Wl,--gc-sections -lm -o "$d/transaction-test-$mode"
 ASAN_OPTIONS=detect_leaks=0 "$d/transaction-test-$mode"
done
