#!/usr/bin/env python3
"""Actual full-row AMP physical label loss/wrong-slot must fail independently."""
import os,signal,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2];out=root/'build/host/drum-amp-caption';out.mkdir(parents=True,exist_ok=True)
s=(root/'firmware/src/app/ui.c').read_text();anchor="physical_stage_label(label,0,'D',matrix_value(r));"
if s.count(anchor)!=1:raise SystemExit('missing/duplicate AMP caption owner')
for name,replacement in [('missing-amp-label','label[0]=0;'),('wrong-amp-slot',"physical_stage_label(label,1,'D',matrix_value(r));")]:
 with tempfile.TemporaryDirectory(prefix='amp-caption-negative-') as d:
  f=Path(d)/'ui.c';f.write_text(s.replace(anchor,replacement))
  env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_drum_amp_graph_test.c',SMC_UI_OUT=str(out/name),X0X_HOST_DEFS=os.environ.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(f)+'"')
  p=subprocess.Popen(['sh','tests/host/smc_ui_test.sh'],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True)
  try:output,_=p.communicate(timeout=100)
  except subprocess.TimeoutExpired:
   try:os.killpg(p.pid,signal.SIGKILL)
   except ProcessLookupError:pass
   output,_=p.communicate();(out/(name+'.log')).write_text(output);raise SystemExit('mutant timeout, not an accepted negative')
  (out/(name+'.log')).write_text(output)
  if p.returncode==0 or 'AMP_PHYSICAL_SLOT_CAPTION_MISSING' not in output or 'FAIL ' not in output:raise SystemExit('wrong runtime rejection '+name+'\n'+output[-2000:])
print('PASS missing/wrong physical AMP slot captions rejected by real pixel oracle')
