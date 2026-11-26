#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D"); T=$1
W=$T; H="$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$H/FLAGS.txt" | xargs)"
cd "$R/ab-latest-headless"
f="$T/dsp/s1c/revsc-proto.inc"
grep -F 'gain=fb+.29f*(1.f-fb)*taper;' "$f"
sed -i 's/fb=s->t\[RVFB\], taper=fb<.5f?fb\*2.f:1.f, gain=fb+.29f\*(1.f-fb)\*taper;/fb=s->t[RVFB], gain=fb;/' "$f"
grep -F 'fb=s->t[RVFB], gain=fb;' "$f"
for candidate in authored-original-pole authored-corrected-pole; do
 if [[ "$candidate" == authored-original-pole ]]; then
   sed -i 's/\.0050506339f/.171572875f/' "$f"; grep -F '.171572875f*' "$f"
 else
   sed -i 's/\.171572875f/.0050506339f/' "$f"; grep -F '.0050506339f*' "$f"
 fi
 cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -I"$W/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen "$W/render.c" firmware/src/app/s1c_part.c "$W/dsp/s1c/"{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$W/factorial-tap"
 ROOM_FEED="$T/original-wet.tap" ROOM_TRACE="$T/$candidate.tap" "$T/factorial-tap" "$R/fw/build/smk37-elite/fdata.img" 113 "$T/$candidate.raw" wet
 rm -- "$T/$candidate.raw"
done
