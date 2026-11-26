#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")/../.." && pwd);cd "$D"; X=$D/sj6-ota-history/transposed-muladd; L=$D/sj6-raw-main-ota; Y=$D/sj6-ota-history/transposed-auth
O=$(mktemp -d "$X/tmp.XXXXXX");trap 'rm -rf "$O"' EXIT;export TMPDIR=$O
python3 "$Y/upstream.py" "$L/physical.c" "$O/fused.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$O/fused.c" > "$O/separate.c"
python3 sj6-ota-history/hoist/wire.py "$O/separate.c" "$O/hoisted.c"
python3 sj6-ota-history/wire.py "$L/physical.c" "$O/packed.c"
{ printf '#include <math.h>\nstatic inline float binary32_madd(float a,float b,float c){volatile float product=a*b;return product+c;}\n#define fmaf binary32_madd\n';cat "$O/hoisted.c";} >"$O/muladd.c"
{ printf '#include <math.h>\n#define fmaf(a,b,c) (((a)*(b))+(c))\n'; cat "$O/hoisted.c"; } > "$O/plain.c"
for n in packed hoisted muladd plain; do
 { cat "$O/$n.c"; echo 'float one(OTA*s,float x,const float*p,const float*c){return physical_tick(s,x,p,c);}'; } > "$O/$n-all.c"
 cc -fPIC -shared -O2 -ffp-contract=off "$O/$n-all.c" -lm -o "$O/$n.so"
done
python3 "$Y/authgen.py" "$O/base.py"
python3 "$X/score.py" "$O/base.py" "$O/auth.py"
export PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench OPENBLAS_NUM_THREADS=1
for c in authored lower; do python3 "$O/auth.py" "$O/hoisted.so" "$O/packed.so" "$O/muladd.so" "$c" >"$X/$c.json";done
python3 "$X/dynamic.py" "$O/hoisted.so" "$O/muladd.so" >"$X/dynamic.json"
python3 "$X/dynamic.py" "$O/muladd.so" "$O/plain.so" --require-equal >"$X/plain-vs-volatile.json"
