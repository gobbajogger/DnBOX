#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd);cd "$D"; X=$D/sj6-ota-history/hoist; Y=$D/sj6-ota-history/transposed-auth; L=$D/sj6-raw-main-ota; W=/tmp/phaser-warm-own.Y0XNbQ/tree
T=$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
O=$(mktemp -d "$X/tmp.XXXXXX");trap 'rm -rf "$O"' EXIT;export TMPDIR=$O
python3 "$Y/upstream.py" "$L/physical.c" "$O/fused.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$O/fused.c" > "$O/separate.c"
cp "$L/physical.c" "$O/original.c"
python3 sj6-ota-history/wire.py "$L/physical.c" "$O/packed.c"
for n in original packed separate; do
 python3 "$X/wire.py" "$O/$n.c" "$O/$n-new.c"
done
FLAGS=(-target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W/experiments/original_synth" -I"$W/firmware/src/dsp" -DEXTERNAL_TANH=1 -include "$D/sj6-ota-history/exact-share.h" -include "$D/sj6-ota-history/tanhreuse/fast.h")
for n in original original-new packed packed-new separate separate-new;do
 cc -fPIC -shared -O2 -ffp-contract=off "$O/$n.c" -lm -o "$O/$n.so"
 { cat "$O/$n.c"; echo "float one(OTA *s,float x,const float *p,const float *c){return physical_tick(s,x,p,c);}"; } > "$O/$n-dyn.c"
 cc -fPIC -shared -O2 -ffp-contract=off "$O/$n-dyn.c" -lm -o "$O/$n-dyn.so"
 python3 "$Y/native.py" "$O/$n.c" "$O/$n-native.c"
 "$T/bin/clang" "${FLAGS[@]}" -c "$O/$n-native.c" -o "$O/$n.o"
 "$T/bin/clang" "${FLAGS[@]}" -S "$O/$n-native.c" -o "$O/$n.s"
 echo "$n"; "$T/bin/nm" -S --size-sort "$O/$n.o"; "$T/bin/nm" -u "$O/$n.o"
 grep -n -E ' / .*\(f\)' "$O/$n.s"
done | tee "$X/native.receipt"
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
python3 "$X/authgen.py" "$O/auth.py"
for c in authored lower;do python3 "$O/auth.py" "$O" "$c" >"$X/$c.json";done
python3 "$X/dynamic.py" "$O" >"$X/dynamic.json"
