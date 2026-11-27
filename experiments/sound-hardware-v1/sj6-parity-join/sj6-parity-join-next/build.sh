#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
P=$PWD/sj6-parity-join-next
BASE=$PWD/sj6-normal-shared-state/tree/firmware/src
IN=$PWD/sj6-mixer-compose-check/app
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cp -ar "$BASE/dsp" "$T"/
cp -ar "$BASE/app" "$T/app"
cp "$IN/"original_runtime.{c,h} "$T/app/"
cp "$BASE/x0x_cold.h" "$T/"
python "$P/wire.py" "$T"
for f in juno6.h juno6_delay_native.inc juno6_unison_native.inc; do
 diff -u --label "a/firmware/src/dsp/$f" --label "b/firmware/src/dsp/$f" "$BASE/dsp/$f" "$T/dsp/$f" || [[ $? == 1 ]]
done > "$P/candidate.patch"
for f in original_runtime.c original_runtime.h; do
 diff -u --label "a/firmware/src/app/$f" --label "b/firmware/src/app/$f" "$IN/$f" "$T/app/$f" || [[ $? == 1 ]]
done >>"$P/candidate.patch"
H=$PWD/s1-render-next/integrated/tree/firmware/src/dsp/synthone/experiment/audiokit-4.9.2/AudioKit/Core/Soundpipe
S=$PWD/fw/tests/fixtures/synthone-factory/fullfx/upstream/AudioKit/Core/Soundpipe/modules/revsc.c
python sj6-room-next/precision/wire.py "$S" "$T/rev20.c" 14
F=(-std=gnu11 -ffunction-sections -fdata-sections -fPIC -O2 -ffp-contract=off -DX0X_HOST=1 -DX0X_DB=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_SOURCE_MIX_WITNESS=1 -DX0X_SJ6_NATIVE_ROOM=1 -DX0X_SJ6_NATIVE_LIMITER=1 -DX0X_SJ6_NATIVE_CLASSIC=1 -DX0X_SJ6_NATIVE_DELAY=1 -DX0X_SJ6_NATIVE_UNISON=1 -DX0X_SJ6_LFO1_DELAY_PORT=1 -DDNBOX_SJ6_SOURCE_UNISON_ORDER=1 -I"$PWD/sj6-room-compose" -I"$T/dsp" -I"$BASE/app" -I"$BASE/../../experiments/original_synth" -I"$BASE/dsp/synthone")
# With the new guard off, input syntax after preprocessing is exact.
cp "$BASE/dsp/juno6.c" "$T/dsp/baseline.c"
cc "${F[@]}" -I"$BASE/dsp" -E -P "$BASE/dsp/juno6.c" >"$T/off0.i"
cc "${F[@]}" -E -P "$T/dsp/baseline.c" >"$T/off1.i"
cmp "$T/off0.i" "$T/off1.i"
cp "$IN/original_runtime.c" "$T/app/base.c"
cc "${F[@]}" -E -P "$T/app/base.c" >"$T/app0.i"
cc "${F[@]}" -E -P "$T/app/original_runtime.c" >"$T/app1.i"
cmp "$T/app0.i" "$T/app1.i"
cat "$P/probe.inc" >>"$T/app/original_runtime.c"
cc "${F[@]}" -DDNBOX_SJ6_JOIN_LAB=1 -c "$T/app/original_runtime.c" -o "$T/runtime.o"
cc "${F[@]}" -DDNBOX_SJ6_JOIN_LAB=1 -c "$T/dsp/juno6.c" -o "$T/juno.o"
cc -fPIC -O2 -ffp-contract=off -c sj6-multirate/sj6_stereo_delay.c -o "$T/delay.o"
cc -shared -fPIC -O2 -ffp-contract=off -I"$H" -I"$H/lib/dr_wav" \
 "$T"/{runtime,juno,delay}.o "$T/rev20.c" sj6-room-compose/room.c sj6-room-next/precision/codec.c -lm -o "$P/owner.so"
