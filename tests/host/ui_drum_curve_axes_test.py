#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual native chart scales plus runtime missing-curve negative."""
import os, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/host/ui-drum-curve-axes'
OUT.mkdir(parents=True,exist_ok=True)
def run(name, source=None):
    env=os.environ.copy()
    env.update(SMC_UI_TEST_FILE='tests/host/ui_drum_curve_axes_test.c',SMC_UI_OUT=str(OUT/name))
    if source: env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
    p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
    (OUT/(name+'.log')).write_text(p.stdout)
    return p
p=run('normal')
if p.returncode: print(p.stdout[-5000:]);raise SystemExit(p.returncode)
if 'PASS native graph time window' not in p.stdout:raise SystemExit('missing positive runtime completion')
with tempfile.TemporaryDirectory(prefix='dnbox-curve-negative-') as tmp:
    d=Path(tmp);ui=d/'ui.c';ui.write_bytes((ROOT/'firmware/src/app/ui.c').read_bytes())
    source=(ROOT/'firmware/src/app/ui_drum_voice.inc').read_text()
    anchor='    cv_poly_aa(points,used/2,col);'
    if source.count(anchor)!=1:raise SystemExit('missing/duplicate production curve anchor')
    (d/'ui_drum_voice.inc').write_text(source.replace(anchor,'    (void)points; /* negative: axes without actual curve */'))
    p=run('missing-curve',ui)
    if p.returncode==0 or 'ink>100' not in p.stdout or 'FAIL ' not in p.stdout:
        print(p.stdout[-5000:]);raise SystemExit('missing curve was not rejected at actual runtime')
print('PASS actual graph axes and runtime missing-curve negative')
