#!/bin/bash
set -euo pipefail
T="$(cd "$(dirname "$0")";pwd)"
W=/workspace/scratch/3a7051e761a0
D="$W/recovered-sound/s1-room-parity"
DONOR="$D/build5"
M="$(dirname "$(find "$W/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)")"
H="$(dirname "$(dirname "$(find "$W/s1-fx-parity/reference-build/closure/experiment" -path '*/AudioKit/Core/Soundpipe/modules/buthp.c' -print -quit)")")"
cd "$W/ab-latest-headless"
read -ra opt <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$W/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls/FLAGS.txt"|xargs)"
cc -std=gnu11 -O2 -ffunction-sections -fdata-sections -include "$DONOR/dsp/s1c/s1c.h" -DS1_WIDEN_ORACLE=1 -DS1_ROOM_EXACT=1 -DS1_ROOM_EXACT_MASTER=1 -DS1_DELAY_EXACT=1 \
 -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opt[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 \
 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 \
 -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -DNO_LIBSNDFILE -I"$D" -I"$M/../lib/faust" -I"$H" -I"$H/lib/dr_wav" -I"$T" \
 -I"$DONOR/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
 "$DONOR/render.c" "$DONOR/part.c" "$DONOR/dsp/s1c/"{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
 "$D/exact-room.c" "$T/exact-delay.c" "$M/"{revsc,compressor,vdelay}.c "$H/modules/"{buthp,base,crossfade,moogladder}.c -Wl,--gc-sections -lm -o "$T/compact"
