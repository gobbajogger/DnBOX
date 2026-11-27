#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
HERE=$PWD; ROOT=$(realpath ..); D=$(realpath "${1:?private ARM64 bench build dir}")
TREE=$ROOT/firmware-fit-next/tree; BASE=$TREE/firmware/src
H=$ROOT/s1-render-next/integrated/tree/firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe
S=$ROOT/fw/tests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe/modules/revsc.c
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
mkdir "$T/app"; ln -s "$BASE/dsp" "$T/dsp"; ln -s "$BASE/x0x_cold.h" "$T/x0x_cold.h"
cp "$BASE/app/original_runtime.c" "$T/app/base.c"
# Keep same private preproc dir for both and stock header paired by same relative path.
python wire.py "$BASE/app" "$T/app"
flags=( -ffunction-sections -fdata-sections -fvisibility=hidden -O3 -ffp-contract=off -fPIC -std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NATIVE_ROOM=1 -I"$HERE" -I"$BASE/app" -I"$BASE/dsp" -I"$TREE/experiments/original_synth" )
cc "${flags[@]}" -E -P "$T/app/base.c" >"$T/base.i"
cc "${flags[@]}" -E -P "$T/app/original_runtime.c" >"$T/guard.i"
cmp "$T/base.i" "$T/guard.i"
sha256sum "$T/base.i"|cut -d' ' -f1
cat probe.inc >>"$T/app/original_runtime.c"
cc "${flags[@]}" -DDNBOX_ROOM_COMPOSE_LAB=1 -c "$T/app/original_runtime.c" -o "$T/runtime.o"
cc "${flags[@]}" -c "$BASE/dsp/juno6.c" -o "$T/juno.o"
for m in 12 14; do
 python ../sj6-room-next/precision/wire.py "$S" "$T/rev$m.c" "$m"
 cc -O3 -ffp-contract=off -fPIC -shared -Wl,--gc-sections -I"$H" -I"$H/lib/dr_wav" "$T/runtime.o" "$T/juno.o" room.c "$T/rev$m.c" ../sj6-room-next/precision/codec.c -lm -o "$D/room-compose-$m.so"
done
diff -u "$BASE/app/original_runtime.c" "$T/app/original_runtime.c" | head -70 || true
