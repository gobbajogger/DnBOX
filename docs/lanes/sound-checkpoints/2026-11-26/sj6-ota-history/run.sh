#!/bin/bash
set -euo pipefail
D=$PWD/sj6-ota-history; L=$PWD/sj6-raw-main-ota
O=$(mktemp -d "$D/tmp.XXXXXX"); trap 'rm -rf "$O"' EXIT
python3 "$D/wire.py" "$L/physical.c" "$O/packed.c"
for x in old new;do
 if [ "$x" = old ];then cp "$L/physical.c" "$O/replay.inc";else cp "$O/packed.c" "$O/replay.inc";fi
 cc -O2 -fPIC -shared -ffp-contract=off -I"$O" "$D/ab.c" -lm -o "$O/$x.so"
done
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
python3 "$D/smoke.py" "$O/old.so" "$O/new.so" >"$D/smoke.json"
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src
python3 "$L/make.py" "$O/f.c"
cc -O2 -fPIC -shared -ffp-contract=off -DX0X_SJ6_OTA_NATIVE=1 -I"$W/dsp" -I"$W" "$O/f.c" -lm -o "$O/f.so"
for c in authored lower;do
 extra=(); [[ $c != lower ]] || extra=(lower)
 python3 "$D/auth-compare.py" "$O/f.so" "$O/new.so" "$O/old.so" "${extra[@]}" > "$D/$c.json"
done
if [[ ${PI32_OBJECT_ONLY:-1} == 1 ]];then
 TC=$PWD/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
 export TMPDIR="$O"
 for p in "$L/physical.c" "$O/packed.c";do
  "$TC/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$TC/include" -c "$p" -o "$O/q.o"
  "$TC/bin/objdump" -h "$O/q.o"
  "$TC/bin/nm" -u "$O/q.o"
 done > "$D/native.receipt"
fi
