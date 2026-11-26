#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D")
P="$R/rehydrated-reverb/dual-room/tree/firmware/src"
S="$(realpath "${1:-$D/donor-tree}")/firmware/src"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
python - "$P/gfx.c" "$T/boot-original.inc" <<'PY'
import sys
from pathlib import Path
p,o=map(Path,sys.argv[1:]);s=p.read_text()
f=s[s.index('static void draw_boot_screen('):]
f=f[:f.index('\n}')+2]
o.write_text(f.replace('draw_boot_screen(','draw_boot_screen_original(',1)+'\n')
PY
cc -std=c11 -O2 -I"$T" -I"$S" -I"$P/../../build/smk37-elite/gen" "$D/boot-test.c" -o "$T/boot-test"
"$T/boot-test"
