#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Reachable real UI journeys plus production regression mutations."""
import os, signal
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/host/ui-no-loss-return'
OUT.mkdir(parents=True,exist_ok=True)
def run(case, name, source=None, mode=None):
    env=os.environ.copy()
    env.update(SMC_UI_TEST_FILE=f'tests/host/{case}_test.c',SMC_UI_OUT=str(OUT/name))
    if mode: env['NOLOSS_CASE']=mode
    if source: env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
    p=subprocess.Popen(['sh','tests/host/smc_ui_test.sh'],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True)
    try: output,_=p.communicate(timeout=150)
    except subprocess.TimeoutExpired:
        try: os.killpg(p.pid,signal.SIGKILL)
        except ProcessLookupError: pass
        output,_=p.communicate()
        (OUT/(name+'.log')).write_text(output)
        raise SystemExit('compile/runtime timeout, not an accepted result')
    p=subprocess.CompletedProcess(p.args,p.returncode,output)
    (OUT/(name+'.log')).write_text(p.stdout)
    return p
cases=[('ui_home_live_return','HOME_LIVE_ROUTE_MISSING',None),('ui_adsr_knob_identity','ADSR_PHYSICAL_KNOB_CAPTION_MISSING',None),('ui_action_state_return','GEN_CHANGED_STATE_INVISIBLE',None)]
for case,marker,mode in cases:
    p=run(case,case)
    if p.returncode or 'PASS ' not in p.stdout: print(p.stdout[-4000:]);raise SystemExit('positive journey failed: '+case)
p=subprocess.run([str(OUT/'ui_action_state_return/test')],env=dict(os.environ,NOLOSS_CASE='modal'),stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=20)
(OUT/'modal.log').write_text(p.stdout)
if p.returncode or 'PASS modal physical cues' not in p.stdout:raise SystemExit('modal journey failed: '+p.stdout)
source=(ROOT/'firmware/src/app/ui.c').read_text()
mutants=[(cases[0], 'ui.home_live = act == ACT_HOME_LIVE;', 'ui.home_live = 0;'),
         (cases[1], 'text_c(31+e.slot[k]*59,y+8-off', 'text_c(31+((e.slot[k]+1)%4)*59,y+8-off'),
         (cases[2], 'gen && ui.gen_stale[b] ? "CHANGED" : 0', '0 ? "CHANGED" : 0')]
for (case,marker,mode),old,new in mutants:
    if source.count(old)!=1:raise SystemExit('mutation source anchor changed: '+old)
    with tempfile.TemporaryDirectory(prefix='dnbox-no-loss-') as tmp:
        path=Path(tmp)/'ui.c';path.write_text(source.replace(old,new))
        p=run(case,case+'-negative',path)
        # The route mutation leaves the list intact, but the real frame does not switch.
        expected=marker if case!='ui_home_live_return' else 'memcmp(matrix_frame,live_frame,sizeof live_frame)'
        if p.returncode==0 or 'FAIL ' not in p.stdout or expected not in p.stdout:
            print(p.stdout[-4000:]);raise SystemExit('production regression did not fail at runtime: '+case)
print('PASS real HOME/ADSR/GEN/modal journeys and production regression negatives')
