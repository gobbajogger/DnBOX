#!/bin/bash
set -eu
W=$(cd "$(dirname "$0")";pwd); R=$(dirname "$W"); D="$R/ab-latest-headless"
H="$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$H/FLAGS.txt" |xargs)"
opts+=(-DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1)
cd "$D"
cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -I"$W/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen "${S1_ROOM_STIMULUS:-$R/s1-controlled-ab/render.c}" firmware/src/app/s1c_part.c "$W/dsp/s1c/"{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$W/$1"
