#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"; R=$(realpath ..)
D=$(mktemp -d);trap 'rm -rf "$D"' EXIT
H=$R/s1-render-next/integrated/tree/firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe
S=$R/fw/tests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe/modules/revsc.c
python wire.py "$S" "$D/rev.c"
for k in 0 1 2; do
 cc -O2 -ffp-contract=off -fno-fast-math -shared -fPIC -I"$H" -I"$H/lib/dr_wav" -DLAYOUT=$k driver.c "$D/rev.c" "$R/sj6-room-next/precision/codec.c" -lm -o "$D/path-$k.so"
done
python score.py "$D" "$R"
