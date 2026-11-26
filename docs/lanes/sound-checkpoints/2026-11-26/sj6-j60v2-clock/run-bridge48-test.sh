#!/bin/sh
set -eu
cd "$(dirname "$0")"
d=$(mktemp -d)
trap 'rm -rf "$d"' EXIT
PYTHONDONTWRITEBYTECODE=1 python3 - <<'PY' >"$d/generated_test_rows.h"
from coefficients import row

def make(a):
    return "{" + ",".join(x.hex()+"f" for x in a) + "}"

up = row(.5,1.)
down = row(0.,.5)
print("#define COEFS {",make(up[:32]),",",make(down[30::-2]),",",down[31].hex()+"f }")
print("static const float exact_up[64] = ", make(up),";")
print("static const float exact_down[64] = ", make(down),";")
PY
cc -std=c99 -Wall -Wextra -Werror -O2 -ffp-contract=off -I"$d" bridge48_test.c -lm -o "$d/test"
"$d/test"
