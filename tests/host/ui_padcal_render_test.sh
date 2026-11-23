#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
OUT=${SMC_UI_OUT:-build/host/actual-padcal}
mkdir -p "$OUT"
python3 - "$OUT" <<'PY'
import pathlib,sys,hashlib,json
out=pathlib.Path(sys.argv[1]);p=pathlib.Path('firmware/src/app/main_fm1.c');s=p.read_text()
def extract(anchor):
    assert s.count(anchor) == 1, anchor
    start = s.index(anchor)
    a = s.index('{', start)
    depth, b = 1, a + 1
    while depth:
        depth += (s[b] == '{') - (s[b] == '}')
        b += 1
    return s[start:b] + '\n'

body = extract('uint8_t smk_padcal_draw(uint32_t w, uint32_t h)')
(out/'actual_padcal.inc').write_text(body)
(out/'actual_padcal_dispatch.inc').write_text(
    extract('void smk_padcal_enter(void)\n{') +
    extract('static void smk_padcal_frame(void)\n{'))
(out/'actual-source.json').write_text(json.dumps({'sourcePath':str(p),'sourceSHA256':hashlib.sha256(p.read_bytes()).hexdigest(),'functionSHA256':hashlib.sha256(body.encode()).hexdigest(),'method':'Unchanged exact function extraction; supplied fixture globals; RAM-only fixture states; no device I/O'},indent=2)+'\n')
PY
SMC_UI_OUT="$OUT" SMC_UI_TEST_FILE=tests/host/ui_padcal_render_test.c sh tests/host/smc_ui_test.sh
