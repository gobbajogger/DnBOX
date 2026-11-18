#!/usr/bin/env python3
"""Wrong/absent Chicago release law must fail the retained matrix behavior oracle."""
import os,signal,subprocess,sys,tempfile,time
from pathlib import Path
def bounded(command,*,timeout,**kw):
 p=subprocess.Popen(command,start_new_session=True,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
 try:
  output,_=p.communicate(timeout=timeout)
  return subprocess.CompletedProcess(command,p.returncode,output)
 except subprocess.TimeoutExpired:
  try:os.killpg(p.pid,signal.SIGKILL)
  except ProcessLookupError:pass
  output,_=p.communicate()
  return subprocess.CompletedProcess(command,124,output)

def timeout_control():
 command=[sys.executable,'-c','import subprocess,time; p=subprocess.Popen(["sleep","60"]); print("CHILD",p.pid,flush=True); time.sleep(60)']
 result=bounded(command,timeout=.2)
 if result.returncode!=124 or 'CHILD ' not in result.stdout:raise SystemExit('timeout control lost output/status')
 pid=int(result.stdout.split('CHILD ')[1].split()[0])
 for _ in range(50):
  try:state=Path('/proc/'+str(pid)+'/stat').read_text().split(') ')[1].split()[0]
  except FileNotFoundError:break
  if state=='Z':break  # killed; awaiting system reaper, not a running descendant
  time.sleep(.01)
 else:raise SystemExit('timeout control left a live child')
 print('PASS timeout kills/reaps owned process group and preserves output')

if '--timeout-control' in sys.argv:
 timeout_control();raise SystemExit(0)
timeout_control()
root=Path(__file__).resolve().parents[2]
out=Path(sys.argv[1]).resolve();source=(root/'firmware/src/app/ui.c').read_text()
anchor='return e->sustain*fm_expf(-t/e->release);'
if source.count(anchor)!=1:raise SystemExit('missing/duplicate Chicago release anchor')
for name,replacement in [('linear-release','return e->sustain*fm_maxf(0.f,1.f-t/e->release);'),('missing-release','return 0;')]:
 with tempfile.TemporaryDirectory(prefix='matrix-release-negative-') as tmp:
  p=Path(tmp)/'ui.c';p.write_text(source.replace(anchor,replacement))
  env=dict(os.environ,SOUND_MATRIX_SINGLE='1',X0X_HOST_DEFS=os.environ.get('X0X_HOST_DEFS','')+' -DX0X_HOST_UI_SOURCE="'+str(p)+'"')
  result=bounded(['sh','tests/host/sound_matrix_test.sh',str(out/name)],cwd=root,env=env,timeout=100)
  (out/(name+'.log')).write_text(result.stdout)
  if result.returncode in (0,124) or 'CHICAGO_RELEASE_LAW_MISMATCH' not in result.stdout or 'FAIL ' not in result.stdout:
   raise SystemExit(name+' not rejected by intended runtime oracle\n'+result.stdout[-3000:])
print('PASS two actual production Chicago release-law negatives')
