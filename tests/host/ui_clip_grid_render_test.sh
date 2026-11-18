#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/actual-clip-grid}
mkdir -p "$OUT"
python3 - "$OUT" <<'PY'
import pathlib,sys,hashlib,json
out=pathlib.Path(sys.argv[1]);p=pathlib.Path('firmware/src/app/main_fm1.c');s=p.read_text()
anchor='uint8_t smk_clip_lcd_draw(uint32_t w, uint32_t h)'
assert s.count(anchor)==1
start=s.index(anchor);a=s.index('{',start);depth=1;b=a+1
while depth:
 depth+=(s[b]=='{')-(s[b]=='}');b+=1
body=s[start:b]+'\n';(out/'actual_clip_grid.inc').write_text(body)
(out/'actual-source.json').write_text(json.dumps({'sourcePath':str(p),'sourceSHA256':hashlib.sha256(p.read_bytes()).hexdigest(),'functionSHA256':hashlib.sha256(body.encode()).hexdigest(),'method':'Unchanged exact function extraction; supplied fixture globals; clip-key subpage entry rejects; no device I/O'},indent=2)+'\n')
PY
SMC_UI_OUT="$OUT" SMC_UI_TEST_FILE=tests/host/ui_clip_grid_render_test.c sh tests/host/smc_ui_test.sh
