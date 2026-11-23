#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/song-text}
mkdir -p "$OUT"
python3 - "$OUT" <<'PY'
import pathlib,sys
s=pathlib.Path('firmware/src/app/ui.c').read_text();start='static UI_DRAW void draw_song(int band)\n{';end='/* Continuous send track'
assert s.count(start)==1 and s.count(end)==1
s=s[s.index(start):s.index(end)]
assert s.count('k ? "END" : "EMPTY   WHITE KEY: ADD BAR"')==1,'Expected current short status and physical action label'
out=pathlib.Path(sys.argv[1])
(out/'song_text_current.inc').write_text(s.replace('draw_song(int band)','observed_draw_song(int band)',1))
(out/'song_text_before.inc').write_text(s.replace('draw_song(int band)','before_draw_song(int band)',1).replace('k ? "END" : "EMPTY   WHITE KEY: ADD BAR"','k ? "END" : "EMPTY: WHITE KEYS ADD BARS"',1))
PY
SMC_UI_OUT="$OUT" SMC_UI_TEST_FILE=tests/host/ui_song_text_test.c sh tests/host/smc_ui_test.sh
