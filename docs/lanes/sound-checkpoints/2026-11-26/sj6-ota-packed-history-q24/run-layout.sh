#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
S=$(mktemp -d)
trap 'rm -rf "$S"' EXIT
python3 "$D/transform.py" "$D/full.c" "$S/packed.c" both
python3 "$D/layout.py" "$S/packed.c" "$S/layout.c"
"${CC:-cc}" -shared -fPIC -O2 -std=c11 -Wall -Wextra -Werror -ffp-contract=off "$S/layout.c" -lm -o "$S/layout.so"
python3 "$D/layout-test.py" "$S/layout.so"
