#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import os,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2];out=root/'build/host/synth-step-footer';out.mkdir(parents=True,exist_ok=True)
def run(name,source=None):
 env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_synth_step_footer_test.c',SMC_UI_OUT=str(out/name))
 if source:env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
 p=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);(out/(name+'.log')).write_text(p.stdout);return p
p=run('positive')
if p.returncode or 'PASS synth parameter' not in p.stdout:raise SystemExit(p.stdout[-4000:])
s=(root/'firmware/src/app/ui.c').read_text()
for name,a,b in [('missing-lane','if(band==2)sound_step_footer();','(void)0;'),('wrong-window','int step=step_of(c),x=8+c*14','int step=c,x=8+c*14')]:
 if s.count(a)!=1:raise SystemExit('missing/duplicate mutation anchor '+name)
 with tempfile.TemporaryDirectory(prefix='synth-step-negative-') as d:
  f=Path(d)/'ui.c';f.write_text(s.replace(a,b));p=run(name,f)
  if p.returncode==0 or 'SYNTH_STEP_NUMBER_MISSING' not in p.stdout or 'FAIL ' not in p.stdout:raise SystemExit('runtime mutant not rejected '+p.stdout[-4000:])
print('PASS synth parameter16-step lane, actual states and two production negatives')
