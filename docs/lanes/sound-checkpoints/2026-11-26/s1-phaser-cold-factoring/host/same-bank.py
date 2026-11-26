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
  run([P/'base/candidate',pack,i,ref,'wet',note,got])
  b=note.read_bytes();assert b==got.read_bytes()
  run([P/'new/candidate',pack,i,ra,'wet',note,got]);assert b==got.read_bytes()
  assert ref.read_bytes()==ra.read_bytes()
  return dict(row=i,pcm_sha256=hashlib.sha256(ra.read_bytes()).hexdigest(),notes_sha256=hashlib.sha256(b).hexdigest())

with ThreadPoolExecutor(int(os.environ.get('JOBS','8'))) as ex:
 for q in ex.map(check,[int(k) for k in sys.argv[1:]] or sorted(want)):
  print(json.dumps(q),flush=True)

