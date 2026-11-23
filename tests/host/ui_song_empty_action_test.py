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
mutants=[
 ('missing-cue','k ? "END" : "EMPTY"','k ? "END" : ""','EMPTY_SONG_ACTION_CUE_MISSING'),
 ('unreadable-selection','k==ui.song_sel?SOUND_SELECT:SOUND_FACE','k==ui.song_sel?C_WHITE:SOUND_FACE','SONG_SELECTED_CONTRAST_MISSING')]
for name,anchor,replacement,marker in mutants:
    if s.count(anchor)!=1:raise SystemExit('missing/duplicate SONG mutation anchor: '+name)
    with tempfile.TemporaryDirectory(prefix='song-action-negative-') as d:
        f=Path(d)/'ui.c';f.write_text(s.replace(anchor,replacement))
        p=run(name,f)
        if p.returncode==0 or marker not in p.stdout or 'FAIL ' not in p.stdout:
            raise SystemExit('SONG mutation not rejected at runtime: '+p.stdout[-4000:])
print('PASS real palette SONG action, missing-cue and unreadable-selection production negatives')
