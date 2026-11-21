#!/usr/bin/env python3
"""Wrong/absent Chicago release law must fail the retained matrix behavior oracle."""
import os,select,signal,subprocess,sys,tempfile,time
from pathlib import Path
def stop_group(process):
 try:os.killpg(process.pid,signal.SIGKILL)
 except ProcessLookupError:pass
 return process.communicate()[0]

def bounded(command,*,timeout,ready=None,**kw):
 p=subprocess.Popen(command,start_new_session=True,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,**kw)
 if ready is not None:
  if not select.select([ready],[],[],30)[0] or ready.read(1)!=b'1':
   raise SystemExit('timeout fixture did not start its descendant: '+stop_group(p))
 try:
  output,_=p.communicate(timeout=timeout)
  return subprocess.CompletedProcess(command,p.returncode,output)
 except subprocess.TimeoutExpired:
  return subprocess.CompletedProcess(command,124,stop_group(p))

def timeout_control():
 # The deadline covers a running process group. Interpreter startup can
 # exceed the deliberately short control deadline on a loaded builder.
 read_fd,write_fd=os.pipe()
 with os.fdopen(read_fd,'rb',buffering=0) as ready,os.fdopen(write_fd,'wb',buffering=0) as writer:
  program=('import os,subprocess,sys,time; p=subprocess.Popen(["sleep","60"]); '
           'print("CHILD",p.pid,flush=True); os.write(int(sys.argv[1]),b"1"); time.sleep(60)')
  command=[sys.executable,'-c',program,str(writer.fileno())]
  result=bounded(command,timeout=.2,ready=ready,pass_fds=(writer.fileno(),))
 if result.returncode!=124 or 'CHILD ' not in result.stdout:raise SystemExit('timeout control lost output/status')
 pid=int(result.stdout.split('CHILD ')[1].split()[0])
 for _ in range(50):
  try:state=Path('/proc/'+str(pid)+'/stat').read_text().split(') ')[1].split()[0]
  except (FileNotFoundError,ProcessLookupError):break
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
