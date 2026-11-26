#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
S=$(mktemp -d)
trap 'rm -rf "$S"' EXIT
L="$D/../sj6-raw-main-ota/physical.c"
python3 "$D/transform.py" "$L" "$S/transpose.c"
sed -E 's/fmaf\(-a,mid,x\)/x-a*mid/;s/fmaf\(-b,y,mid\)/mid-b*y/' "$S/transpose.c" > "$S/separate.c"
"${CC:-cc}" -shared -fPIC -O2 -ffp-contract=off "$L" -lm -o "$S/oracle.so"
for v in transpose separate; do
    "${CC:-cc}" -shared -fPIC -O2 -ffp-contract=off "$S/$v.c" -lm -o "$S/$v.so"
    OPENBLAS_NUM_THREADS=1 python3 "$D/test.py" "$S" "$v.so"
done
