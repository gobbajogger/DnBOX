#!/usr/bin/env bash
set -euo pipefail
d="$(cd "$(dirname "$0")" && pwd)"
r="${DNBOX_AB_WORKSPACE:-$(cd "$d/../.." && pwd)}"
h="${DNBOX_AB_HANDOFF:-$r/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls}"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$h/FLAGS.txt" | xargs)"
cd "$d"
for m in 0 1 2; do
  python setup.py "$m"
  cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen render.c firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o render
  p="$r/fw/build/smk37-elite/fdata.img"
  ./render "$p" 62 "$m-compact.f32le" wet
  FULL_RATE=1 ./render "$p" 62 "$m-full.f32le" wet
done
python analyze.py
