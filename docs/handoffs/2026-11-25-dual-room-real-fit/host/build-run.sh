#!/usr/bin/env bash
set -euo pipefail
d="$(cd "$(dirname "$0")" && pwd)"
r="${DNBOX_AB_WORKSPACE:-$(cd "$d/.." && pwd)}"
h="${DNBOX_AB_HANDOFF:-$r/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls}"
latest="${DNBOX_AB_LATEST:-$r/ab-latest-headless}"
for arm in recovered; do
  src="$r/fw"; opts=()
  if [[ $arm == recovered ]]; then
    src="$latest"
    read -ra opts <<<"$(grep -o -- '-DX0X_[A-Z0-9_]*=1' "$r/all-controls-remote-join/compile-flags.txt" | xargs)"
    opts+=(-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DX0X_IMA_TABLES_DEFINE=1)
  fi
  mkdir -p "$d/$arm"
  (
    cd "$src"
    cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen "$d/render.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$d/$arm/render"
    p="$r/fw/build/smk37-elite/fdata.img"
    for i in 1 62 113 128 278 435; do
      for mode in wet dry; do "$d/$arm/render" "$p" "$i" "$d/$arm/$i-$mode.f32le" "$mode"; done
    done
  )
done
