#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/list-unit}
mkdir -p "$OUT"
python3 - "$OUT" <<'PY'
import pathlib,sys
s=pathlib.Path('firmware/src/app/ui.c').read_text();start='static UI_DRAW void draw_list(int band)\n{';end='static UI_DRAW void draw_ask('
assert s.count(start)==1 and s.count(end)==1
s=s[s.index(start):s.index(end)]
a='cv_text(ux, base_y(nf, ny, &FONT_XS), &FONT_XS, unit, sel ? C_WHITE : C_GRAY);'
assert s.count(a)==1,'Expected selected-unit foreground correction'
out=pathlib.Path(sys.argv[1]);out.joinpath('list_unit_current.inc').write_text(s.replace('draw_list(int band)','observed_draw_list(int band)',1))
out.joinpath('list_unit_before.inc').write_text(s.replace('draw_list(int band)','before_draw_list(int band)',1).replace(a,a.replace('sel ? C_WHITE : C_GRAY','C_GRAY'),1))
PY
SMC_UI_OUT="$OUT" SMC_UI_TEST_FILE=tests/host/ui_list_unit_test.c sh tests/host/smc_ui_test.sh
