#!/bin/bash
set -euo pipefail
r=$(pwd); d="$r/s1-focus-keycache/host-compare"; w="$r/s1-fx-parity"
cd "$r/s1-focus-keycache/tree"
f=(-std=c11 -ffunction-sections -fdata-sections -fno-builtin -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1
   -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen)
f+=( $(cat ../compose-flags.txt) )
for opt in Oz O2; do
  x=(); if [[ $opt == Oz ]]; then x=(-UDNBOX_S1_KEY_CACHE); fi; for s in s1c s1u; do cc -Oz "${f[@]}" "${x[@]}" -c firmware/src/dsp/s1c/$s.c -o "$d/$s-$opt.o"; done
  cc -Oz "${f[@]}" "${x[@]}" "$w/compact.c" "$d/ima.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/s1c_front.c firmware/src/dsp/s1pack/s1pack.c "$d/s1c-$opt.o" "$d/s1u-$opt.o" -Wl,--gc-sections -lm -o "$d/$opt"
done
for cue in 62 113 151 154 191 366; do
  for mode in wet delay; do
    for opt in Oz O2; do "$d/$opt" "$r/fw/build/smk37-elite/fdata.img" "$cue" "$d/tmp-$opt" "$mode" >/dev/null; done
    cmp "$d/tmp-Oz" "$d/tmp-O2"
    printf '%d %s bit-exact %s\n' "$cue" "$mode" "$(sha256sum "$d/tmp-O2" | cut -d' ' -f1)"
  done
done
