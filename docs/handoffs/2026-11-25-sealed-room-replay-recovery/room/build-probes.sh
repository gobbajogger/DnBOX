#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
cd "$1"
readarray -t flags < <(tr ' ' '\n' < "$R/all-controls-remote-join/compile-flags.txt" | sed '/^$/d')
opts=(-DX0X_ORIGINAL_SYNTH=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 "${flags[@]}" -I firmware/src -I build/smk37-elite/gen -I experiments/original_synth -I experiments/wobble)
cc -std=c11 -O2 -Wl,--gc-sections -ffunction-sections -fdata-sections -DX0X_HOST "${opts[@]}" "$D/fixture.c" firmware/src/app/{original_sound,s1c_part}.c -lm -o "$D/fixture"
"$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -S -O2 "${opts[@]}" "$D/room-offsets.c" -o "$D/room-offsets.s"
"$D/fixture" layout
