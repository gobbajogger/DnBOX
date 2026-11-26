#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
S=$(mktemp -d)
trap 'rm -rf "$S"' EXIT
"${CC:-cc}" -shared -fPIC -O2 -ffp-contract=off "$D/full.c" -lm -o "$S/full.so"
for pack in up both; do
  python3 "$D/transform.py" "$D/full.c" "$S/$pack.c" "$pack"
  "${CC:-cc}" -shared -fPIC -O2 -ffp-contract=off "$S/$pack.c" -lm -o "$S/$pack.so"
  OPENBLAS_NUM_THREADS=1 python3 "$D/test.py" "$S" "$pack.so"
done
