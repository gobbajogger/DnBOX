#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D"); cd "$R"
T=s1-organ-wet-recovery/native-widen-bfp/tree
A=recovered-sound/s1-room-parity/unified-releasephase
python "$D/extract.py"
read -ra flags <<<"$(xargs < all-controls-remote-join/compile-flags.txt)"
cc -std=c11 -O2 -ffp-contract=off -ffunction-sections -fdata-sections -DX0X_HOST -DX0X_SUPERJ6_FACTORY=1 -DX0X_ORIGINAL_SYNTH=1 "${flags[@]}" -I"$T/firmware/src/app" -I"$T/firmware/src/dsp/synthone/sequencer" -I"$T/experiments/original_synth" -Is1-fx-parity -I"$D/private" "$D/emit.c" -Wl,--gc-sections -lm -o "$D/private/emit"
cp "$A/tuning-load.h" "$D/private/"
python s1-oracle-tape/build-ref.py "$D/private"
