#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual drum audition-state visibility and bounded redraw regression."""
import os, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/host/key-sound-state'
OUT.mkdir(parents=True,exist_ok=True)
def run(name, source=None):
    env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_key_sound_state_test.c',SMC_UI_OUT=str(OUT/name))
    if source: env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
    try:p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120)
    except subprocess.TimeoutExpired as error:
        output=error.stdout or b''
        (OUT/(name+'.log')).write_text(output.decode(errors='replace') if isinstance(output,bytes) else output)
        raise SystemExit('timeout is not a runtime negative PASS: '+name)
    (OUT/(name+'.log')).write_text(p.stdout)
    (OUT/(name+'.exit')).write_text(str(p.returncode)+'\n')
    return p
p=run('positive')
if p.returncode or 'PASS all22 drum' not in p.stdout:raise SystemExit(p.stdout[-4000:])
s=(ROOT/'firmware/src/app/ui_drum_voice.inc').read_text()
for name,anchor in [
 ('missing-generic-cue','if(p==0 && !key_sounds())cv_text(91,ly,&FONT_T,"KEY SOUND OFF",gray);'),
 ('missing-cp-stage-cue','if(!key_sounds())cv_text(91,28-off,&FONT_T,"KEY SOUND OFF",SOUND_GRAY);')]:
    if s.count(anchor)!=1:raise SystemExit('missing/duplicate key-state mutation anchor: '+name)
    d=OUT/(name+'-source');d.mkdir(exist_ok=True)
    f=d/'ui.c';f.write_bytes((ROOT/'firmware/src/app/ui.c').read_bytes());(d/'ui_drum_voice.inc').write_text(s.replace(anchor,'(void)0; /* missing state */'))
    p=run(name,f)
    if p.returncode==0 or 'KEY_SOUND_OFF_STATE_MISSING' not in p.stdout or 'FAIL ' not in p.stdout:
        raise SystemExit('missing cue not rejected at runtime: '+name+' '+p.stdout[-4000:])
    print('PASS production runtime missing-state negative: '+name,flush=True)
print('PASS drum audition state and separately anchored generic/CP-stage missing-state negatives')
