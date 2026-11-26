#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/../..";R=$PWD; X="$R/sj6-ota-history/pitchenv-lease";W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp;T="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
O=$(mktemp -d "$X/tmp.XXXXXX");trap 'rm -rf "$O"' EXIT; export TMPDIR="$O"
mkdir -p "$O/firmware/src/dsp"; cp "$W/dnbx_core.h" "$O/firmware/src/dsp/dnbx_core.h"; cd "$O"
patch --fuzz=0 -s -p1 < "$X/pitch-layout.patch"
F=$(cat "$R/all-controls-remote-join/compile-flags.txt")
for j in 0 1; do
  cp "$W/dnbx_core.h" "$O/owner.h"; if [[ $j == 1 ]]; then cp "$O/firmware/src/dsp/dnbx_core.h" "$O/owner.h";fi
  "$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -isystem "$T/include" -I"$O" -I"$W" -I"$W/.." $F -DDNBX_DIAGNOSTIC_SJ6_PITCH_LEASE="$j" -c "$X/layout.c" -o "$O/probe.o"
  "$T/bin/nm" -S "$O/probe.o" | awk '$3=="R"' > "$O/$j.nm"
done
cmp "$O/0.nm" "$O/1.nm"; cat "$O/1.nm"
