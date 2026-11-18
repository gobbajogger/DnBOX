#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual empty SONG action, assignment, and missing-cue regression."""
import os, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/host/song-empty-action'
OUT.mkdir(parents=True,exist_ok=True)
def run(name, source=None):
    env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_song_empty_action_test.c',SMC_UI_OUT=str(OUT/name))
    if source: env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
    p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
    (OUT/(name+'.log')).write_text(p.stdout)
    return p
p=run('positive')
if p.returncode or 'PASS empty SONG' not in p.stdout:raise SystemExit(p.stdout[-4000:])
s=(ROOT/'firmware/src/app/ui.c').read_text()
anchor='k ? "END" : "EMPTY: WHITE KEYS ADD BARS"'
if s.count(anchor)!=1:raise SystemExit('missing/duplicate SONG mutation anchor')
with tempfile.TemporaryDirectory(prefix='song-cue-negative-') as d:
    f=Path(d)/'ui.c';f.write_text(s.replace(anchor,'k ? "END" : "EMPTY"'))
    p=run('missing-cue',f)
    if p.returncode==0 or 'EMPTY_SONG_ACTION_CUE_MISSING' not in p.stdout or 'FAIL ' not in p.stdout:
        raise SystemExit('missing cue not rejected at runtime: '+p.stdout[-4000:])
print('PASS empty SONG real action and missing-cue production negative')
