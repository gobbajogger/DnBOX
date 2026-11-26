#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd)
W=/tmp/phaser-warm-own.Y0XNbQ/tree
T=$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
O=$(mktemp -d "$D/sj6-ota-history/preflight.XXXXXX"); trap 'rm -rf "$O"' EXIT; export TMPDIR=$O
python3 "$D/sj6-ota-history/wire.py" "$D/sj6-raw-main-ota/physical.c" "$O/packed.c"
python3 "$(dirname "$0")/split.py" "$O/packed.c" "$O/tick.c" "$O/split.c"
for n in tick split; do
 "$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W/experiments/original_synth" -I"$W/firmware/src/dsp" -DEXTERNAL_TANH=1 -include "$D/sj6-ota-history/exact-share.h" -include "$D/sj6-ota-history/tanhreuse/fast.h" -c "$O/$n.c" -o "$O/$n.o"
 echo "$n"; "$T/bin/nm" -S --size-sort "$O/$n.o"; "$T/bin/nm" -u "$O/$n.o"
done
cc -Os -fPIC -shared -ffp-contract=off "$O/"{tick,split}.c -lm -o "$O/test.so"
python3 "$(dirname "$0")/split-test.py" "$O/test.so"
