import numpy as np
import os,json,multiprocessing as mp, subprocess as sp,tempfile,sys,hashlib
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
P=Path(__file__).resolve().parent; W=Path('/workspace/scratch/3a7051e761a0')
A=W/'recovered-sound/s1-room-parity'; R=A/'compressor-cache'
want={q['id']:q for q in map(json.loads,(R/'bank.jsonl').read_text().splitlines()[1:])}
T=A/'tuning-tables'; tuning=json.loads((T/'index.json').read_text())
waves=W/'s1-fx-parity/reference-build/factory-inputs/public-morphs.f32'
commands=A/'aligned-commands.bin';pack=W/'recovered-sound/s1-bank-replay/original-private/pack.private'
assert len(want)==906
def check(i):
 with tempfile.TemporaryDirectory(dir=P) as h:
  f=Path(h);note=f/'wanted';got=f/'got';data=f/'raw.f32'
  env={x:y for x,y in os.environ.items() if x not in {'S1_AUTHORED_TUNING_F32','S1_AUTHORED_TUNING_NPO','ROOM_TRACE','ROOM_FEED','ROOM_RETURN','WIDEN_TRACE','WIDTH_TRACE'}}
  env['S1_EXACT_MORPH_WAVES']=str(waves)
  if (t:=tuning.get(str(i))) is not None:
   file=T/t['file'];assert hashlib.sha256(file.read_bytes()).hexdigest()==t['sha256']
   env.update(S1_AUTHORED_TUNING_F32=str(file),S1_AUTHORED_TUNING_NPO=str(t['npo']))
  def run(args):sp.run(list(map(str,args)),env=env,capture_output=True,check=True,timeout=60)
  run([R/'original',waves,commands,i,os.devnull,'wet',note])
  ref,ra=f/'ref.f32',f/'ab.f32'
  proc=sp.run(list(map(str,[P/'base/candidate',pack,i,ref,'wet',note,got])),env=env,capture_output=True,check=True,timeout=60)
  b=note.read_bytes();assert b==got.read_bytes()
  assert hashlib.sha256(ref.read_bytes()).hexdigest()==want[i]['trial_sha256']
  run([P/'fast/candidate',pack,i,ra,'wet',note,got]);assert b==got.read_bytes()
  x=np.fromfile(ref,dtype='<f4').reshape(-1,2);y=np.fromfile(ra,dtype='<f4').reshape(-1,2)
  assert x.shape==y.shape and np.isfinite(y).all()
  e=y.astype(np.float64)-x
  def db(q):return 20*np.log10(max(q,1e-30))
  def rms(q):return float(np.sqrt(np.mean(np.square(q,dtype=np.float64))))
  def metrics(x,y,e):
   return dict(null_dbr=db(rms(e)/max(rms(x),1e-30)),err_dbfs=db(rms(e)),peak_dbfs=db(float(abs(e).max())),level_db=db(rms(y))-db(rms(x)))
  o=dict(row=i,whole=metrics(x,y,e),tail=metrics(x[-44100:],y[-44100:],e[-44100:]),exact=not np.any(e),notes_sha256=hashlib.sha256(b).hexdigest())
  if not o['exact']:
   X=abs(np.fft.rfft(x,axis=0));Y=abs(np.fft.rfft(y,axis=0));E=abs(np.fft.rfft(e,axis=0))
   fs=np.fft.rfftfreq(len(x),1/44100)
   o['bands']=[dict(lo=l,hi=h,ref_dbfs=db(rms(X[(fs>=l)&(fs<h)])*np.sqrt(2/len(x))),delta_db=db(rms(Y[(fs>=l)&(fs<h)]))-db(rms(X[(fs>=l)&(fs<h)])),null_dbr=db(rms(E[(fs>=l)&(fs<h)])/max(rms(X[(fs>=l)&(fs<h)]),1e-30))) for l,h in ((20,200),(200,2000),(2000,5000),(5000,16000))]
  o['angle']=[[int(j),int(count),float.fromhex(lo),float.fromhex(hi),int(skip)] for tag,j,count,lo,hi,skip in (r.split() for r in proc.stderr.decode().splitlines() if r.startswith('PHCOS '))]
  return o

with ThreadPoolExecutor(int(os.environ.get('JOBS','8'))) as ex:
 for q in ex.map(check,[int(k) for k in sys.argv[1:]] or sorted(want)):
  print(json.dumps(q),flush=True)
  assert q['whole']['peak_dbfs']<0
