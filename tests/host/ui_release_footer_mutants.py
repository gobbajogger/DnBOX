#!/usr/bin/env python3
"""Released renderer must retain the entire lane and its real held-step marker."""
import os,signal,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2]
base=root/'build/host/release-footer-mutants';base.mkdir(parents=True,exist_ok=True)
out=Path(tempfile.mkdtemp(prefix='run-',dir=base));(base/'latest-path.txt').write_text(str(out)+'\n')
s=(root/'firmware/src/app/ui.c').read_text()
for name,old,new in [
 ('missing-lane','if(band==2)sound_step_footer();','if(0)sound_step_footer();'),
 ('missing-held','if(enabled && step==ui.held_step)cv_rect(x,236-KNOB_Y,12,1,SOUND_INK);','if(0)cv_rect(x,236-KNOB_Y,12,1,SOUND_INK);')]:
 if s.count(old)!=1:raise SystemExit('missing/duplicate mutation anchor '+name)
 with tempfile.TemporaryDirectory(prefix='release-footer-negative-') as d:
  source=Path(d)/'ui.c';source.write_text(s.replace(old,new))
  env=dict(os.environ,X0X_HOST_DEFS=f'-I{root}/firmware/src/app -DX0X_HOST_UI_SOURCE="{source}"')
  destination=out/name
  if destination.exists():raise SystemExit('refuse stale mutation output '+str(destination))
  p=subprocess.Popen(['sh','tests/host/original_release_ui_test.sh',str(destination)],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True)
  try:log,_=p.communicate(timeout=180)
  except subprocess.TimeoutExpired:
   try:os.killpg(p.pid,signal.SIGKILL)
   except ProcessLookupError:pass
   log,_=p.communicate();(out/(name+'.wrapper.log')).write_text(log);raise SystemExit('timeout is not a negative pass')
  (out/(name+'.wrapper.log')).write_text(log)
  runtime=destination/'normal.log';text=runtime.read_text() if runtime.exists() else ''
  if p.returncode==0 or 'SOUND_STEP_FOOTER_MISMATCH' not in text or 'FAIL line ' not in text:
   raise SystemExit('missing intended runtime rejection '+name+'\n'+text[-2000:]+log[-1000:])
print('PASS actual release UI rejects missing lane and missing held marker')
