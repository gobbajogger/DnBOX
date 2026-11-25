#!/bin/sh
set -eu
root=$(realpath "$1"); w=$(realpath "$2"); d=$(realpath "$(dirname "$0")")
tc="$root/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
"$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -I"$w/src/firmware/src" -I"$w/src/firmware/hal" \
  -I"$w/src/experiments/original_synth" -I"$w/src/build/smk37/gen" -idirafter "$tc/include" -DBOARD_SMK37=1 -DX0X_PROJECT_NATIVE_CONFIG=1 \
  -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 \
  -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -DX0X_S1_BEND_RANGE_PROBE=1 \
  -DX0X_S1_MOD_RATE_PROBE=1 -DX0X_S1_AUTOPAN_PROBE=1 -DX0X_S1_AUTOPAN_PORT=1 -O0 -S -emit-llvm "$d/pan-layout.c" -o "$w/pan-layout.ll"
grep '= constant i32' "$w/pan-layout.ll"
