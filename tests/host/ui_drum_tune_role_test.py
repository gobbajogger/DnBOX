#!/usr/bin/env python3
import os,signal,subprocess,tempfile
from pathlib import Path
root=Path(__file__).resolve().parents[2];out=root/'build/host/drum-tune-role';out.mkdir(parents=True,exist_ok=True)
def run(name,source=None):
 env=dict(os.environ,SMC_UI_TEST_FILE='tests/host/ui_drum_tune_role_test.c',SMC_UI_OUT=str(out/name))
 if source:env['X0X_HOST_DEFS']=env.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(source)+'"'
 p=subprocess.Popen(['sh','tests/host/smc_ui_test.sh'],cwd=root,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True)
 try:output,_=p.communicate(timeout=100)
 except subprocess.TimeoutExpired:
  try:os.killpg(p.pid,signal.SIGKILL)
  except ProcessLookupError:pass
  output,_=p.communicate();(out/(name+'.log')).write_text(output)
  raise SystemExit(name+' timed out; owned group killed/reaped')
 (out/(name+'.log')).write_text(output)
 return subprocess.CompletedProcess(p.args,p.returncode,output)

p=run('positive')
if p.returncode or 'PASS BD Tune' not in p.stdout:raise SystemExit(p.stdout[-3000:])
s=(root/'firmware/src/app/ui_drum_voice.inc').read_text()
for name,a,b,marker in [('name-overrides-role','role == DG_NONE && d && name_is(d, "Tune")','d && name_is(d, "Tune")','BD_TUNE_RASTER_MISMATCH'),('missing-live-marker','cv_rect(left + x - 1, y - 1, 3, 3, C_WHITE);','(void)x;(void)y;','BD_TUNE_LIVE_MARKER_MISSING')]:
 if s.count(a)!=1:raise SystemExit('missing/duplicate anchor '+name)
 with tempfile.TemporaryDirectory(prefix='drum-role-negative-') as d:
  f=Path(d)/'ui.c';f.write_bytes((root/'firmware/src/app/ui.c').read_bytes());(Path(d)/'ui_drum_voice.inc').write_text(s.replace(a,b));p=run(name,f)
  if not p.returncode or marker not in p.stdout or 'FAIL ' not in p.stdout:raise SystemExit(name+' not rejected as intended '+p.stdout[-3000:])
print('PASS native BD sweep semantics/live marker, retained pitch fork and two production negatives')
