#!/bin/bash
set -euo pipefail
D=$PWD/sj6-ota-history; X=$PWD/sj6-ota-history/tanhreuse; L=$PWD/sj6-raw-main-ota
O=$(mktemp -d "$D/tmp.XXXXXX"); trap 'rm -rf "$O"' EXIT
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src
python3 "$D/wire.py" "$L/physical.c" "$O/packed.c"
P=/tmp/phaser-warm-own.Y0XNbQ/tree/experiments/original_synth
for x in old new;do
 if [ "$x" = old ];then cp "$L/physical.c" "$O/replay.inc";else cp "$O/packed.c" "$O/replay.inc";fi
 if [[ $x == new ]]; then
   cc -O2 -fPIC -ffp-contract=off -I"$P" -c "$P/fused_reference.c" -o "$O/fused.o"
   cc -O2 -fPIC -shared -ffp-contract=off -DTEST_CALLS -I"$O" -I"$P" -I"$D" -include "$D/exact-share.h" -include "$X/fast.h" -I"$W/dsp" "$X/ab.c" "$O/fused.o" -lm -o "$O/$x.so"
 else
   cc -O2 -fPIC -shared -ffp-contract=off -I"$O" "$X/ab.c" -lm -o "$O/$x.so"
 fi
done
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
python3 "$X/smoke.py" "$O/old.so" "$O/new.so" >"$X/shared-smoke.json"
W=/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src
python3 "$L/make.py" "$O/f.c"
cc -O2 -fPIC -shared -ffp-contract=off -DX0X_SJ6_OTA_NATIVE=1 -I"$W/dsp" -I"$W" "$O/f.c" -lm -o "$O/f.so"
for c in authored lower;do
 extra=(); [[ $c != lower ]] || extra=(lower)
 python3 "$X/auth.py" "$O/f.so" "$O/new.so" "$O/old.so" "${extra[@]}" > "$X/shared-$c.json"
done
if [[ ${PI32_OBJECT_ONLY:-1} == 1 ]];then
 TC=$PWD/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
 export TMPDIR="$O"
 for ht in 0 1;do
  "$TC/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$TC/include" -I"$P" -I"$W/dsp" -DEXTERNAL_TANH=$ht -include "$D/exact-share.h" -include "$X/fast.h" -c "$O/packed.c" -o "$O/q.o"
  "$TC/bin/objdump" -h "$O/q.o"
  "$TC/bin/nm" -u "$O/q.o"
 done > "$X/shared-native.receipt"
fi
