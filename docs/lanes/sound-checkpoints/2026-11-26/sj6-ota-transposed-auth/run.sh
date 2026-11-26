#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd);cd "$D"; X=$D/sj6-ota-history/transposed-auth; L=$D/sj6-raw-main-ota; W=/tmp/phaser-warm-own.Y0XNbQ/tree
T=$D/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
O=$(mktemp -d "$X/tmp.XXXXXX");trap 'rm -rf "$O"' EXIT;export TMPDIR=$O
python3 "$X/upstream.py" "$L/physical.c" "$O/fused.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$O/fused.c" > "$O/separate.c"
python3 sj6-ota-history/wire.py "$L/physical.c" "$O/packed.c"
for n in packed fused separate;do
 cc -fPIC -shared -O2 -ffp-contract=off "$O/$n.c" -lm -o "$O/$n.so"
 python3 "$X/native.py" "$O/$n.c" "$O/$n-native.c"
 "$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W/experiments/original_synth" -I"$W/firmware/src/dsp" -DEXTERNAL_TANH=1 -include "$D/sj6-ota-history/exact-share.h" -include "$D/sj6-ota-history/tanhreuse/fast.h" -c "$O/$n-native.c" -o "$O/$n.o"
 "$T/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -ffunction-sections -fdata-sections -isystem "$T/include" -I"$W/experiments/original_synth" -I"$W/firmware/src/dsp" -DEXTERNAL_TANH=1 -include "$D/sj6-ota-history/exact-share.h" -include "$D/sj6-ota-history/tanhreuse/fast.h" -S "$O/$n-native.c" -o "$O/$n.s"
 echo "$n"; "$T/bin/nm" -S --size-sort "$O/$n.o"; "$T/bin/nm" -u "$O/$n.o"
done | tee "$X/native.receipt"
for n in packed fused separate;do
 echo "$n"
 sed -n '/^physical_tick:/,/^[[:space:]]*[.]size[[:space:]]*physical_tick/p' "$O/$n.s" | grep -E 'paired_fma|fsub|fmul| = .* [*] | = .* - ' || true
done > "$X/disasm.receipt"

export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
cc -fPIC -shared -O2 -ffp-contract=off "$O/separate.c" "$O/separate-native.c" -lm -o "$O/state.so"
python3 "$X/lifecycle.py" "$O/state.so" > "$X/lifecycle.receipt"
if [[ ${OBJECT_ONLY:-0} == 1 ]];then exit 0;fi
python3 "$X/authgen.py" "$O/auth.py"
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
for c in authored lower;do python3 "$O/auth.py" "$O/packed.so" "$O/fused.so" "$O/separate.so" "$c" >"$X/$c.json";done
