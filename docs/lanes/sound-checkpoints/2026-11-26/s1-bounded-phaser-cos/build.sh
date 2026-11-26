#!/bin/bash
set -euo pipefail
D="/workspace/scratch/3a7051e761a0/recovered-sound/s1-room-parity"; R="/workspace/scratch/3a7051e761a0"; A=$D
C=$(realpath "$1"); T=$(realpath "$2"); SRC="$R/ab-latest-headless"
MOD=$(find "$R/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)
M=$(dirname "$MOD")
HP=$(find "$R/s1-fx-parity/reference-build/closure/experiment" -path '*/AudioKit/Core/Soundpipe/modules/buthp.c' -print -quit)
H=$(dirname "$(dirname "$HP")")
L="$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$L/FLAGS.txt"|xargs)"
cd "$C"
sha256sum render.c part.c dsp/s1c/s1c.c dsp/s1c/s1c.h >"$T/donor-source.sha256"
cd "$SRC"
cc -std=gnu11 -O2 -ffunction-sections -fdata-sections -include "$C/dsp/s1c/s1c.h" \
     -DS1_ROOM_EXACT=1 -DS1_ROOM_EXACT_MASTER=1 -DS1_DELAY_EXACT=1 -DS1_NOISE_EXACT=1 -DS1_FILTER_EXACT=1 \
     -DS1_MORPH_EXACT=1 -DS1_AUX_EXACT=1 -DS1_CRUSH_EXACT=1 -DS1_STEREO_EXACT=1 -DS1_LFO_EXACT=1 -DS1_PITCH_EXACT=${EXACT_PITCH:-0} -DS1_CRUSH_SOURCE_FLOAT=${EXACT_CRUSH_FLOAT:-0} -DS1_ENVELOPE_EXACT=${EXACT_ENV:-0} \
     -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 \
     -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 \
     -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -DNO_LIBSNDFILE -I"$A" -I"$M/../lib/faust" -I"$H" -I"$H/lib/dr_wav" \
     -I"$C/dsp/s1c" -I"$C/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
     "$T/render.c" "$C/part.c" "$T/s1c.c" "$C/dsp/s1c/"{s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c \
     "$A/"{exact-room,exact-delay,exact-filter,exact-morph,exact-aux,exact-stereo,source-crusher-rate}.c \
     -DDNBOX_S1_COMP_COEF_CACHE=1 -DDNBOX_S1_COMP_ONE_LOG=1 "$T/compressor-cache.c" "$M/"{revsc,vdelay,pan2}.c "$T/phaser.c" -DDNBOX_PH_COEF=1 ${PHFLAGS:-} \
     "$H/modules/"{buthp,butbp,base,crossfade,moogladder,ftbl,osc,fosc}.c -Wl,--gc-sections -lm -o "$T/candidate"
