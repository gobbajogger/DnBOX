#!/bin/bash
set -eu
Q=$(cd "$(dirname "$0")"; pwd); S="$1/firmware/src"; B="$2"; V="$3"
python "$Q/wire.py" "$S/app/s1c_part.c" > "$Q/part.inc"
cc -std=gnu11 -O2 -fPIC -ffunction-sections -fdata-sections -shared -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=0 $(cat "$V") -ffp-contract=off -fno-fast-math -I"$S" -I"$S/app" -I"$B" -I"$1/build/smk37/gen" "$Q/test.c" "$B/dnbank.c" "$S/app/original_sound.c" "$S/dsp/s1c/"{s1c,s1u,s1c_front}.c "$S/dsp/s1pack/s1pack.c" -Wl,--gc-sections,--version-script="$Q/exports" -lm -o "$Q/render.so"
