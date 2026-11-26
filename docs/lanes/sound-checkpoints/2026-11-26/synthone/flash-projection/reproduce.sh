#!/bin/bash
set -euo pipefail
R=$(pwd); O=${1:?output directory}; mkdir -p "$O"; O=$(realpath "$O")
T=$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
D=$R/recovered-sound/s1-room-parity
C=$D/build-releasephase/dsp/s1c
MOD=$(find "$R/s1-fx-parity/reference-build" -path '*/AudioKit/Core/Soundpipe/modules/revsc.c' -print -quit)
M=$(dirname "$MOD")
HP=$(find "$R/s1-fx-parity/reference-build/closure/experiment" -path '*/AudioKit/Core/Soundpipe/modules/buthp.c' -print -quit)
H=$(dirname "$(dirname "$HP")")
L=$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$L/FLAGS.txt"|xargs)"
K=$R/ab-latest-headless
common=(-target pi32v2 -ffunction-sections -fdata-sections -fno-builtin -mcpu=r3 -mfprev1 -ffp-contract=off -I"$T/include" -I"$H" -I"$H/lib/dr_wav" -I"$M/../lib/faust" -I"$D" -DNO_LIBSNDFILE -DORIGINAL_USE_SOFT_FMA)
for m in baseline matched; do
  S=0; [[ $m != matched ]] || S=1
  defs=(-DS1_WIDEN_ORACLE=1 -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 "${opts[@]}")
  for flag in ROOM_EXACT ROOM_EXACT_MASTER DELAY_EXACT NOISE_EXACT FILTER_EXACT MORPH_EXACT AUX_EXACT CRUSH_EXACT STEREO_EXACT LFO_EXACT PITCH_EXACT CRUSH_SOURCE_FLOAT ENVELOPE_EXACT; do defs+=("-DS1_$flag=$S"); done
  "$T/bin/clang" "${common[@]}" -Oz -I"$C/.." -I"$K/firmware/src" -I"$K/firmware/src/app" -I"$K/firmware/src/dsp" -I"$K/build/gen" "${defs[@]}" -c "$C/s1c.c" -o "$O/$m-core.o"
  "$T/bin/clang" "${common[@]}" -Oz -I"$C/.." -I"$K/firmware/src/dsp" "${defs[@]}" -c "$C/s1u.c" -o "$O/$m-s1u.o"
done
for f in "$D/"{exact-delay,exact-room,exact-stereo,exact-filter,exact-aux,exact-morph,source-crusher-rate}.c "$M/"{revsc,compressor,vdelay,pan2,phaser}.c "$H/modules/"{buthp,butbp,crossfade,moogladder,base,ftbl,osc,fosc}.c; do
  "$T/bin/clang" "${common[@]}" "${FX_OPT:--Os}" -c "$f" -o "$O/$(basename "$f" .c).o"
done
s=()
for file in "$O/"{exact-delay,exact-room,exact-stereo}.o; do
 while read -r sym; do s+=(-u "$sym"); done < <(readelf -sW "$file" | awk '$4=="FUNC" && $5=="GLOBAL" && $7!="UND"{print $8}')
done
"$T/bin/ld" -r --gc-sections "${s[@]}" -o "$O/fx-subgraph.o" "$O/"{exact-delay,exact-room,exact-stereo,revsc,compressor,vdelay,pan2,phaser,buthp,butbp,crossfade,moogladder,base}.o
u=()
while read -r sym; do u+=(-u "$sym"); done < <(readelf -sW "$O/baseline-core.o"|awk '$4=="FUNC"&&$5=="GLOBAL"&&$7!="UND"{print $8}')
closure=("$O/"{exact-delay,exact-room,exact-stereo,exact-filter,exact-aux,exact-morph,source-crusher-rate,revsc,compressor,vdelay,pan2,phaser,buthp,butbp,crossfade,moogladder,base,ftbl,osc,fosc}.o)
for m in baseline matched; do
 ext=(); [[ $m != matched ]] || ext=("${closure[@]}")
 "$T/bin/ld" -r --gc-sections "${u[@]}" -o "$O/$m-engine.o" "$O/$m-core.o" "$O/$m-s1u.o" "${ext[@]}"
 "$T/bin/ld" -r --gc-sections "${u[@]}" -o "$O/$m-engine-with-runtime.o" "$O/$m-core.o" "$O/$m-s1u.o" "${ext[@]}" --start-group "$T/lib/r3/"{libm,libc,libcompiler-rt}.a --end-group
done
python "$(dirname "$0")/score.py" "$O"
