#!/usr/bin/env bash
set -euo pipefail
r=$1; cd "$(dirname "$0")"
h="$r/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$h/FLAGS.txt" | xargs)"
for kind in stock cubic cosine; do
 mkdir -p "$kind"
 for arm in full iso; do
  opts2=(); [[ $arm != iso ]] || opts2=(iso)
  python window.py "$r" "$kind" "${opts2[@]}"
  cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen render.c firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o render
  if [[ $arm == full ]]; then
    WARM_PROBE=1 ./render "$r/fw/build/smk37-elite/fdata.img" 62 /dev/null wet | grep warm-probe > "$kind-warm.txt"
  fi
  for i in 1 62 113 128 278 435; do
    ./render "$r/fw/build/smk37-elite/fdata.img" "$i" "$kind/$i-${arm}-wet.f32le" wet
    [[ $arm == iso ]] || ./render "$r/fw/build/smk37-elite/fdata.img" "$i" "$kind/$i-dry.f32le" dry
    mv "$kind/$i-full-wet.f32le" "$kind/$i-wet.f32le" 2>/dev/null || :
  done
 done
 ln -sfn "$kind" recovered
 python measure.py "$r/restored-s1-ab-private/corrected-originals" calibration.json > "$kind.json"
done
