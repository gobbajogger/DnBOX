#!/usr/bin/env python3
"""Both legacy caption/status tests must reject a missing or misnumbered lane."""
import os,signal,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
out=root/'build/host/sound-footer-oracle-negatives';out.mkdir(parents=True,exist_ok=True)
source=(root/'firmware/src/app/ui.c').read_text()
def run(name,script,path):
 env=dict(os.environ,X0X_HOST_DEFS=f'-DX0X_HOST_UI_SOURCE="{path}"')
 p=subprocess.Popen(['sh',f'tests/host/{script}_test.sh',str(out/name)],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True)
 try:log,_=p.communicate(timeout=150)
 except subprocess.TimeoutExpired:
  try:os.killpg(p.pid,signal.SIGKILL)
  except ProcessLookupError:pass
  log,_=p.communicate();(out/(name+'.log')).write_text(log);raise SystemExit('timeout is not a negative pass')
 (out/(name+'.log')).write_text(log)
 if p.returncode==0 or 'SOUND_STEP_FOOTER_MISMATCH' not in log or 'FAIL ' not in log:
  raise SystemExit('missing intended runtime rejection '+name+'\n'+log[-2000:])
mutations=[('missing','if(band==2)sound_step_footer();','if(0)sound_step_footer();'),('wrong-number','char number[8];put_i(number,step+1);','char number[8];put_i(number,step+2);')]
with tempfile.TemporaryDirectory(prefix='sound-footer-negative-') as d:
 for name,old,new in mutations:
  if source.count(old)!=1:raise SystemExit('missing/duplicate production mutation '+name)
  p=Path(d)/(name+'.c');p.write_text(source.replace(old,new))
  for script in ['matrix_knob_caption','chicago_readout']:run(name+'-'+script,script,p)
print('PASS both exact footer oracles reject missing lane and wrong absolute numbers')
