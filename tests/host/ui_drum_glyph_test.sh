#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Drum part-page glyphs: dispatch per control and live motion on a hit.
set -eu
cd "$(dirname "$0")/../.."
SMC_UI_OUT=${1:-build/ui-drum-glyph} SMC_UI_TEST_FILE=tests/host/ui_drum_glyph_test.c sh tests/host/smc_ui_test.sh
# Real 808 curve markers must follow the source-observed time and level.
OUT=${1:-build/ui-drum-glyph}
NEG="$OUT/missing-circuit-marker"
mkdir -p "$NEG"
python3 - "$NEG" <<'PYTHON'
import pathlib,sys
out=pathlib.Path(sys.argv[1])
out.joinpath('ui.c').write_bytes(pathlib.Path('firmware/src/app/ui.c').read_bytes())
s=pathlib.Path('firmware/src/app/ui_drum_voice.inc').read_text()
a='        cv_rect(x-1,y-1,3,3,SOUND_INK);'
if s.count(a)!=1: raise SystemExit('missing/duplicate real circuit marker anchor')
out.joinpath('ui_drum_voice.inc').write_text(s.replace(a,'        (void)x; (void)y; /* negative: missing actual marker */'))
PYTHON
MUTANT=$(cd "$NEG" && pwd)/ui.c
set +e
X0X_HOST_DEFS="${X0X_HOST_DEFS:-} -DX0X_HOST_UI_SOURCE=\"$MUTANT\"" SMC_UI_TEST_FILE=tests/host/ui_drum_glyph_test.c SMC_UI_OUT="$NEG" sh tests/host/smc_ui_test.sh > "$OUT/missing-circuit-marker.log" 2>&1
rc=$?
set -e
[ "$rc" != 0 ] || { echo 'FAIL: missing circuit marker accepted'; exit 1; }
grep -Fq '808_LIVE_MARKER_MISSING' "$OUT/missing-circuit-marker.log" || { cat "$OUT/missing-circuit-marker.log"; exit 1; }
echo 'PASS actual 808 marker absence rejected at source-state framebuffer assertion'
