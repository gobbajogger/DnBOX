#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
S=$(mktemp -d)
trap 'rm -rf "$S"' EXIT
python3 "$D/transform.py" "$D/full.c" "$S/prev.c" both
python3 "$D/aligned.py" "$D/full.c" "$S/next.c"
"${CC:-cc}" -shared -fPIC -O2 -ffp-contract=off "$S/prev.c" -lm -o "$S/full.so"
"${CC:-cc}" -shared -fPIC -O2 -ffp-contract=off "$S/next.c" -lm -o "$S/next.so"
OPENBLAS_NUM_THREADS=1 python3 "$D/test.py" "$S" next.so | python3 -c 'import json,sys; j=json.load(sys.stdin); assert all(r["baseline_sha256"]==r["candidate_sha256"] for r in j["results"]); print(j["summary"]["words"])'
