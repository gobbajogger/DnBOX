#!/bin/bash
set -eu
here="$(cd "$(dirname "$0")" && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf -- "$tmp"' EXIT
base="$1/firmware/src/dsp/drum808.c"; cand="$2/firmware/src/dsp/drum808.c"
python3 - "$base" "$cand" "$tmp/merged.c" <<'PY'
from pathlib import Path
import sys
b,c,m=map(Path,sys.argv[1:]); s=b.read_text(); t=c.read_text()
i=t.index("/* If all six phases"); j=t.index("__attribute__((noinline)) static void bank_block",i)
m.write_text(s+"\n"+t[i:j])
PY
cc -O2 -std=c99 -ffp-contract=off -ffunction-sections -I"$(dirname "$base")" -I"$tmp" -Wl,--gc-sections -DD8_SOURCE='"merged.c"' "$here/owner48-zero-bank-window-check.c" -o "$tmp/check"
"$tmp/check"
