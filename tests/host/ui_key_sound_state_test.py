#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual drum audition-state visibility and bounded redraw regression."""
import os, subprocess, tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/host/key-sound-state'
OUT.mkdir(parents=True,exist_ok=True)
def run(name, source=None):
    env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_key_sound_state_test.c',SMC_UI_OUT=str(OUT/name))
    if source: env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
    p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
    (OUT/(name+'.log')).write_text(p.stdout)
    return p
p=run('positive')
if p.returncode or 'PASS all22 drum' not in p.stdout:raise SystemExit(p.stdout[-4000:])
s=(ROOT/'firmware/src/app/ui_drum_voice.inc').read_text()
anchor='if(p==0 && !key_sounds())cv_text(91,ly,&FONT_T,"KEY SOUND OFF",gray);'
if s.count(anchor)!=1:raise SystemExit('missing/duplicate key-state mutation anchor')
with tempfile.TemporaryDirectory(prefix='song-cue-negative-') as d:
    f=Path(d)/'ui.c';f.write_bytes((ROOT/'firmware/src/app/ui.c').read_bytes());(Path(d)/'ui_drum_voice.inc').write_text(s.replace(anchor,'(void)0; /* missing state */'))
    p=run('missing-cue',f)
    if p.returncode==0 or 'KEY_SOUND_OFF_STATE_MISSING' not in p.stdout or 'FAIL ' not in p.stdout:
        raise SystemExit('missing cue not rejected at runtime: '+p.stdout[-4000:])
print('PASS drum audition state and production missing-state negative')
