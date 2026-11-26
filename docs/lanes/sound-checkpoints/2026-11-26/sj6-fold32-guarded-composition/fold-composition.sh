#!/bin/bash
set -eu
cd "$(dirname "$0")"
N=$(cat fold-workpath); R=$(cat fused-workpath); D="$N/tree/firmware/src/dsp"
f=(-O2 -std=c99 -ffp-contract=off -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -I"$D")
cc "${f[@]}" "$N/fold-stock.c" fold-composition.c -lm -o "$N/stock0"
cc "${f[@]}" "$R/tree/firmware/src/dsp/dnbx_core.c" fold-composition.c -lm -o "$N/stockL"
cc "${f[@]}" -DSJ6_WAVE_PROBE=1 "$N/fold-no-low.c" fold-composition.c -lm -o "$N/fold0"
cc "${f[@]}" "$D/dnbx_core.c" fold-composition.c -lm -o "$N/foldL"
expect_eq() { cmp <("$N/$1" "$3" "$4" "$5") <("$N/$2" "$3" "$4" "$5"); }
for w in 0 1 2 3; do
 for r in 0 1 2; do
 # Sub-only: Fold must never change independently sourced Sub, including Low.
 expect_eq stockL foldL "$w" 1 "$r"
 if [ "$r" -ne 0 ]; then expect_eq fold0 foldL "$w" 2 "$r"; fi
 # Main-only: adding Low law must never change Fold32 behavior.
 expect_eq fold0 foldL "$w" 0 "$r"
 if [ "$w" -ge 2 ];then
   expect_eq stockL foldL "$w" 0 "$r"
   expect_eq stockL foldL "$w" 2 "$r"
 else
   if cmp -s <("$N/stockL" "$w" 0 "$r") <("$N/foldL" "$w" 0 "$r");then exit 9;fi
 fi
 done
done
if cmp -s <("$N/fold0" 0 1 0) <("$N/foldL" 0 1 0);then exit 7;fi
echo "PASS all four shapes × Low/Center/High: signed Sub-only unchanged vs Low baseline, proven Low shift active, Main Fold unchanged by Low, unaffected Saw/PWM and Center/High mix bit-exact during held notes + bend + release, new Sine/Triangle route active."
