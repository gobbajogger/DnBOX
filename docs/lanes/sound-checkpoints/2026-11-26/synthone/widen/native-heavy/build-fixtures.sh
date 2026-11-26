#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D"); cd "$R"
T="$R/s1-organ-wet-recovery/native-widen-bfp/tree"
readarray -t flags < <(tr ' ' '\n' < all-controls-remote-join/compile-flags.txt | sed '/^$/d')
opts=(-DX0X_ORIGINAL_SYNTH=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 "${flags[@]}" -I "$T/firmware/src" -I "$T/build/smk37-elite/gen" -I "$T/experiments/original_synth" -I "$T/experiments/wobble")
cc -std=c11 -O2 -Wl,--gc-sections -ffunction-sections -fdata-sections -DX0X_HOST "${opts[@]}" "$D/fixture.c" "$T"/firmware/src/app/{original_sound,s1c_part}.c -lm -o "$D/fixture"
"$D/fixture" layout
"$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -S -O2 "${opts[@]}" "$D/widen-offsets.c" -o "$D/widen-offsets.s"
