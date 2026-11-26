#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
T="$R/sj6-hp-exact-exp-fit/kept-elite-1"; B="$R/sj6-cross-preset"; O=$(mktemp -d "$D/host.XXXX")
trap 'rm -rf "$O"' EXIT
mkdir -p "$O/firmware/src/dsp"
cp "$T/firmware/src/dsp/juno6_j60_native.inc" "$O/firmware/src/dsp/"
patch -d "$O" -s --fuzz=0 -p1 <"$D/experimental.patch"
cp "$O/firmware/src/dsp/juno6_j60_native.inc" "$O/native-donor.inc"
F=$(tr " " "\n" < "$B/flags-source.txt"|grep -E "^-DX0X_(SJ6|SUPERJ6)" | grep -vE "ZERO_LEAF|CUTOFF_LATCH|SCALE_NATIVE|BEND_TYPED|BEND_RANGE|DNBOX_TYPED|NATIVE_RANDOM2|RANDOM5_6|SEQ_DEST_NATIVE|ARP_ADAPTER_NATIVE|VELOCITY_SETTINGS")
for V in 0 1; do
 cc -shared -fPIC -fvisibility=hidden -O2 -ffp-contract=off -DDNBX_EXPERIMENT_J60_POST="$V" -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 $F -I"$T/firmware/src/dsp" -I"$T/firmware/src" -I"$T/experiments/original_synth" -I"$O" "$D/fixture.c" -lm -o "$O/fixture-$V.so"
done
export OPENBLAS_NUM_THREADS=1 PYTHONPATH=/tmp/sj6-owner-packages:"$R/ab-effects-readonly/sj6-fullbench/bench" SJ6_BIN="$R/ab-effects-readonly/SJ6-AU.private"
python3 "$D/fixture.py" "$O"
