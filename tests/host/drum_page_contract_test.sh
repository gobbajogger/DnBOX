#!/bin/sh
# Production mutation controls for the independent drum page inventory.
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/drum-page-contract}
mkdir -p "$OUT"
for kind in omit swap compact hole; do
 NEG="$OUT/$kind"
 mkdir -p "$NEG"
 python3 - "$kind" "$NEG/ui.c" <<'PY'
import pathlib,sys
s=pathlib.Path('firmware/src/app/ui.c').read_text()
anchors={
 'omit': ('if(drum_amp_ref(r)){amp=r;continue;}', 'if(t==T_909 && v==0 && i==4)continue; if(drum_amp_ref(r)){amp=r;continue;}'),
 'swap': ('"Tune", "Tone", "Attack", "Sound", "Decay", "Snappy", "Tail", "P.Dpth", "Pitch",', '"Attack", "Tone", "Tune", "Sound", "Decay", "Snappy", "Tail", "P.Dpth", "Pitch",'),
 'compact': ('if(drum_amp_ref(r)){amp=r;continue;}', 'if(0 && drum_amp_ref(r)){amp=r;continue;}'),
 'hole': ('add_page(emit, nm, snd[i], i + 1 < n ? snd[i + 1] : NONE, i + 2 < n ? snd[i + 2] : NONE, i + 3 < n ? snd[i + 3] : NONE);', 'add_page(emit, nm, NONE, snd[i], i + 2 < n ? snd[i + 2] : NONE, i + 3 < n ? snd[i + 3] : NONE);'),
}
a,b=anchors[sys.argv[1]]
if s.count(a)!=1: raise SystemExit('production mutation anchor changed')
pathlib.Path(sys.argv[2]).write_text(s.replace(a,b))
PY
 MUTANT=$(cd "$NEG" && pwd)/ui.c
 set +e
 X0X_HOST_DEFS="${X0X_HOST_DEFS:-} -DX0X_HOST_UI_SOURCE=\"$MUTANT\"" SMC_UI_OUT="$NEG" sh tests/host/smc_ui_test.sh > "$OUT/$kind.log" 2>&1
 rc=$?
 set -e
 [ "$rc" != 0 ] || { echo "FAIL: production $kind mutation accepted"; exit 1; }
 grep -q DRUM_PAGE_CONTRACT_MISMATCH "$OUT/$kind.log" || { cat "$OUT/$kind.log"; exit 1; }
done
# The relocated full-width graph must still animate from the real AMP owner.
NEG="$OUT/static-amp"
mkdir -p "$NEG"
python3 - "$NEG/ui.c" <<'PYTHON'
import pathlib,sys
s=pathlib.Path('firmware/src/app/ui.c').read_text()
a='if(v.active && (v.hold_active || v.ramp_valid)) {'
if s.count(a)!=1: raise SystemExit('live AMP mutation anchor changed')
pathlib.Path(sys.argv[1]).write_text(s.replace(a,'if(0 && v.active && (v.hold_active || v.ramp_valid)) {'))
PYTHON
MUTANT=$(cd "$NEG" && pwd)/ui.c
set +e
X0X_HOST_DEFS="${X0X_HOST_DEFS:-} -DX0X_HOST_UI_SOURCE=\"$MUTANT\"" SMC_UI_TEST_FILE=tests/host/ui_drum_glyph_test.c SMC_UI_OUT="$NEG" sh tests/host/smc_ui_test.sh > "$OUT/static-amp.log" 2>&1
rc=$?
set -e
[ "$rc" != 0 ] || { echo 'FAIL: static AMP mutation accepted'; exit 1; }
grep -Fq 'memcmp(idle,first,sizeof idle)' "$OUT/static-amp.log" || { cat "$OUT/static-amp.log"; exit 1; }
echo 'PASS drum page contract: four layout mutations and static full-row AMP rejected'
