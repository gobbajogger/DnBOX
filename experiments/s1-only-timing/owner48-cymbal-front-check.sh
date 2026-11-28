#!/bin/bash
# usage: check-cy-two-pass.sh BASELINE_TREE CANDIDATE_TREE
set -eu
a="$1/firmware/src/dsp/drum808.c"
b="$2/firmware/src/dsp/drum808.c"
dsp="$2/firmware/src/dsp"
here="$(cd "$(dirname "$0")" && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf -- "$tmp"' EXIT
python3 - "$a" "$b" "$tmp/merged.c" <<'PY'
from pathlib import Path
import sys
a,b,m=map(Path,sys.argv[1:])
p=a.read_text(); q=b.read_text()
x=q.index('/* The two BQ3/opamp streams')
y=q.index('/* ---- hi-hats',x)
z=p.index('/* ---- hi-hats',p.index('__attribute__((noinline)) static int cy_run('))
m.write_text(p[:z]+q[x:y].replace('cy_run(', 'cy_compare(')+p[z:])
PY
cc -O2 -std=c99 -ffp-contract=off -ffunction-sections -I"$dsp" -I"$tmp" -Wl,--gc-sections -DD8_SOURCE='"merged.c"' "$here/owner48-cymbal-front-check.c" -o "$tmp/check"
"$tmp/check"
