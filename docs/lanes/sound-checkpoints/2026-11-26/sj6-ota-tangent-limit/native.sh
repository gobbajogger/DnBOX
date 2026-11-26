#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd); cd "$D"; X=$D/sj6-ota-history/tangent; W=/tmp/phaser-warm-own.Y0XNbQ/tree
T=$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
O=$(mktemp -d "$X/tmp.XXXXXX"); trap 'rm -rf "$O"' EXIT; export TMPDIR=$O
python3 sj6-ota-history/wire.py sj6-raw-main-ota/physical.c "$O/packed.c"
python3 "$X/wire.py" "$O/packed.c" "$O/tangent.c"
python3 "$X/native.py" "$O/tangent.c" "$O/split.c"
"$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W/experiments/original_synth" -include "$D/sj6-ota-history/exact-share.h" -c "$O/split.c" -o "$O/split.o"
"$T/bin/nm" -S --size-sort "$O/split.o"
"$T/bin/nm" -u "$O/split.o"
cc -Os -fPIC -shared -ffp-contract=off "$O/"{tangent,split}.c -Wl,--allow-multiple-definition -lm -o "$O/pair.so"
python3 "$X/state-test.py" "$O/pair.so"
