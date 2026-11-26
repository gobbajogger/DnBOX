#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D")
P="$R/rehydrated-reverb/dual-room/tree/firmware/src"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
python - "$P/gfx.c" "$T/boot-candidate.inc" <<'PY'
import sys
from pathlib import Path
p,o=map(Path,sys.argv[1:]);s=p.read_text()
f=s[s.index('static void draw_boot_screen('):]
f=f[:f.index('\n}')+2]
changes={
'static void draw_boot_screen(':'static void draw_boot_screen_sliced(',
'band<2':'band<3',
'cv_begin(240,120':'cv_begin(240,80',
'for (unsigned y=0; y<BOOT_PIC_H; ++y)':'for (unsigned y=band?(unsigned)band*80u-20u:0u; y<BOOT_PIC_H && y+20u<(unsigned)(band+1)*80u; ++y)',
'band*120':'band*80',
'band*120u':'band*80u',
}
for a,b in changes.items():f=f.replace(a,b)
assert 'band<3' in f and 'cv_begin(240,80' in f and '120' not in f
o.write_text(f+'\n')
PY
cc -std=c11 -O2 -I"$T" -I"$P" -I"$P/../../build/smk37-elite/gen" "$D/boot-test.c" -o "$T/boot-test"
"$T/boot-test"
