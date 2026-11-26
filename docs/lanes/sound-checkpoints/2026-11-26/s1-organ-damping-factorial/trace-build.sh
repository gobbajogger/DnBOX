#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D"); T=$1
P="$R/s1-fx-parity"; REF="$P/reference-build"
C="$R/s1-reverb-independent"
cp -r "$C/dsp" "$T/dsp"
cp "$P"/{build_original.py,original.cpp,stimulus.h} "$T/"
cp "$(find "$REF" -name source_fx.hpp|head -1)" "$T/source_fx.hpp"
cp "$R/s1-controlled-ab/render.c" "$T/render.c"
python "$D/wire.py" "$T"
python "$T/build_original.py" "$REF" "$T/original-tap"
W="$T" ; H="$R/restored-handoff/docs/handoffs/2026-11-24-source/composed-controls"
read -ra opts <<<"$(grep -o -- '-DX0X_S1[A-Z0-9_]*=1' "$H/FLAGS.txt" |xargs)"
cd "$R/ab-latest-headless"
build_compact(){
cc -std=c11 -O2 -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_ORIGINAL_SYNTH=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 "${opts[@]}" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_FEEDBACK_PROBE=1 -DORIGINAL_USE_SOFT_FMA -ffp-contract=off -fno-fast-math -I"$W/dsp" -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen "$W/render.c" firmware/src/app/s1c_part.c "$W/dsp/s1c/"{s1c,s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -Wl,--gc-sections -lm -o "$W/compact-tap"
}
build_compact
ROOM_TRACE="$T/original-wet.tap" "$T/original-tap" "$REF/factory-inputs/public-morphs.f32" "$REF/commands/preset-commands.bin" 113 "$T/original-wet.raw" wet
ROOM_FEED="$T/original-wet.tap" ROOM_TRACE="$T/compact-matched.tap" "$T/compact-tap" "$R/fw/build/smk37-elite/fdata.img" 113 "$T/compact-matched.raw" wet
(cd "$T"; printf '%s  %s\n' 09d3fcf1ed1c92bc441b3c1c37dcda796c99ec3b6b08b6e8769205cd1e079716 original-wet.tap 694c6c84cfbb3c881fd606efa05b88e000fc6cdff382ab3bc8e286cd19e0916a compact-matched.tap | sha256sum -c -)
f="$T/dsp/s1c/revsc-proto.inc"
test "$(grep -Fc '.171572875f' "$f")" = 1
sed -i 's/\.171572875f/.0050506339f/' "$f"
build_compact
ROOM_FEED="$T/original-wet.tap" ROOM_TRACE="$T/compact-corrected.tap" "$T/compact-tap" "$R/fw/build/smk37-elite/fdata.img" 113 "$T/compact-corrected.raw" wet
