#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(cd "$D/../.." && pwd)
T=${1:?new output directory};mkdir "$T"; T=$(realpath "$T")
W="$R/s1-organ-wet-recovery/widen-lab"; SRC="$R/ab-latest-headless"
cp -a "$W/dsp" "$T/dsp"; cp "$W/render.c" "$T/render.c"; cp "$SRC/firmware/src/app/s1c_part.c" "$T/part.c"
python "$D/wire-host.py" "$T"
# Verified pinned source closure of the original Synth One reference.
MOD=$(find "$R/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)
M=$(dirname "$MOD")
HP=$(find "$R/s1-fx-parity/reference-build/closure/experiment" -path '*/AudioKit/Core/Soundpipe/modules/buthp.c' -print -quit)
H=$(dirname "$(dirname "$HP")")
test -r "$H/soundpipe.h" && test -r "$M/compressor.c"
L="$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$L/FLAGS.txt"|xargs)"
cd "$SRC"
for arm in compact room room-master room-delay room-delay-master; do
  exact=0; master=0; delay=0
  if [[ $arm != compact ]]; then exact=1; fi
  if [[ $arm == *-master ]]; then master=1; fi
  if [[ $arm == room-delay* ]]; then delay=1; fi
  cc -std=gnu11 -O2 -ffunction-sections -fdata-sections -include "$T/dsp/s1c/s1c.h" -DS1_WIDEN_ORACLE=1 -DS1_ROOM_EXACT=$exact -DS1_ROOM_EXACT_MASTER=$master -DS1_DELAY_EXACT=$delay \
     -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 \
     -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 \
     -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -DNO_LIBSNDFILE -I"$D" -I"$M/../lib/faust" -I"$H" -I"$H/lib/dr_wav" \
     -I"$T/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
     "$T/render.c" "$T/part.c" "$T/dsp/s1c/"{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
     "$D/"{exact-room,exact-delay}.c "$M/"{revsc,compressor,vdelay}.c "$H/modules/"{buthp,base,crossfade,moogladder}.c -Wl,--gc-sections -lm -o "$T/$arm"
done
