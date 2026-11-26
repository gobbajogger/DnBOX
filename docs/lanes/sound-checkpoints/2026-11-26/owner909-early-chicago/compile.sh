#!/bin/bash
set -euo pipefail
d=$(cd "$(dirname "$0")";pwd); r=$(dirname "$d")
P=/tmp/phaser-warm-own.Y0XNbQ/tree
C="$r/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
printf '%s  %s\n' 92a7d2f68d027c40306a27bf954ea7f5a736e14cc8d169a97ef9b30e56cb4f54 "$P/firmware/src/app/engine.c" | sha256sum -c -
cp "$P/firmware/src/app/engine.c" "$d/probe.c"
trap 'rm -f "$d/probe.c" "$d/probe.o"' EXIT
patch -s --fuzz=0 "$d/probe.c" "$d/diagnostic.patch"
for mode in 0 1; do
  TMPDIR="$d" "$C/bin/clang" -target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$C/include" -I"$P/firmware/src/app" -I"$P/firmware/src" -I"$P/build/smk37-elite/gen" -I"$P/firmware/hal" -I"$P/experiments/original_synth" -I"$P/experiments/wobble" -DBOARD_SMK37=1 -DX0X_CORE_PLAN=4 $(cat "$r/all-controls-remote-join/compile-flags.txt") -DX0X_DIAG_OWNER909_EARLY_CHICAGO="$mode" -DX0X_S1C=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_SJ6_NATIVE_J60=1 -DX0X_ARP_HOOKS=1 -DX0X_COLD_COMPANION_LAB=1 -DX0X_909_HUFF_DIAG=1 -DX0X_TANH_SPLIT_LAB=1 -DX0X_S1_PHASER_NATIVE_TEST=1 -DX0X_REMOTE_TYPED_SYNTH=1 -DX0X_REMOTE_DNR3=1 -c "$d/probe.c" -o "$d/probe.o"
  "$C/bin/nm" -S "$d/probe.o" | grep -E 'bass_prepare_job|early_909|early_bass|render_sub|core2_jobs|run_job'
done
