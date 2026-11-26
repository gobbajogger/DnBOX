import concurrent.futures as cf, hashlib, json, os, pathlib, subprocess as s, sys, tempfile, enum
import numpy as np
P=pathlib.Path(__file__).resolve().parent; W=pathlib.Path(os.environ.get('W','/workspace/scratch/3a7051e761a0'))
A=W/'recovered-sound/s1-room-parity'; R=A/'compressor-cache'; B=W/'recovered-sound/s1-bank-replay/original-private'
waves=W/'s1-fx-parity/reference-build/factory-inputs/public-morphs.f32'; ref=A/'aligned-commands.bin'
table=json.loads((A/'aligned-bank.json').read_text()); old={i['id']:i for i in map(json.loads,(R/'bank.jsonl').read_text().splitlines()[1:])}
assert hashlib.sha256(ref.read_bytes()).hexdigest()==table['aligned_sha256']
T=A/'tuning-tables'; tunes=json.loads((T/'index.json').read_text())
ids=sorted(set([1,62,113,128,151,154,163,191,275,289,366,386,434,487,515,596,742,752,802,900,903]+[17+k*41 for k in range(22)]))
named={i for i, r in enumerate(table['rows']) if r['label'] in ['Irresistible Forces Pad','Dark Theater Organ','BB Kubrick Hall Drone','Deeper Space Pad']}
ids=sorted(set(ids)|named)
if os.environ.get('S1_NAMED_ONLY'):ids=sorted(named)
db=lambda a,b: (10*np.log10(np.maximum(a,1e-40)/np.maximum(b,1e-40))).tolist()
pow=lambda a: np.mean(abs(a.astype('f8'))**2,axis=0)
def pair(a,b):
  a,b=a.astype('f8'),b.astype('f8')
  x=pow(a);y=pow(b)
  return dict(reference_rms=np.sqrt(x).tolist(),level=db(y,x),null=db(pow(b-a),x))
def bands(a,b):
  a=a[-44100:]; b=b[-44100:]
  w=np.hanning(len(a))[:,None]
  def fft(x):return np.fft.rfft(x*w,axis=0)
  a,b=fft(a),fft(b); f=np.fft.rfftfreq(len(w),1/44100)
  def bp(x):return np.mean(abs(x)**2,axis=0)
  return [{'band':f'{lo}-{hi}', 'level':db(bp(b[k]),bp(a[k])),'null':db(bp((b-a)[k]),bp(a[k])),'rms':np.sqrt(bp(a[k])).tolist(), 'ref_band_to_window_db':db(np.sum(abs(a[k])**2,axis=0),np.sum(abs(a)**2,axis=0)), 'codec_error_to_window_db':db(np.sum(abs((a-b)[k])**2,axis=0),np.sum(abs(a)**2,axis=0)), 'reference_dbfs':db(2*np.sum(abs(a[k])**2,axis=0)/(len(w)*np.sum(w*w)),1.), 'codec_error_dbfs':db(2*np.sum(abs((a-b)[k])**2,axis=0)/(len(w)*np.sum(w*w)),1.)} for lo,hi in [(20,300),(300,2400),(2400,5000),(5000,16000)] for k in [(f>=lo)&(f<hi)]]
def run(cmd,env):
 return s.run(list(map(str,cmd)),env=env,capture_output=True,check=True,timeout=60)
def record(i,widths,trace=False):
 env={k:v for k,v in os.environ.items() if k not in ['S1_AUTHORED_TUNING_NPO','S1_AUTHORED_TUNING_F32','ROOM_TRACE','ROOM_FEED','ROOM_RETURN','WIDTH_TRACE','WIDEN_TRACE']};env['S1_EXACT_MORPH_WAVES']=str(waves)
 t=tunes.get(str(i))
 if t:
  f=T/t['file'];assert hashlib.sha256(f.read_bytes()).hexdigest()==t['sha256']
  env.update(S1_AUTHORED_TUNING_F32=str(f),S1_AUTHORED_TUNING_NPO=str(t['npo']))
 with tempfile.TemporaryDirectory(dir=P) as f:
  f=pathlib.Path(f);tape=f/'notes'; ev=f/'seen'; tmp=f/'scratch';tp=f/'trace'
  run([R/'original',waves,ref,i,os.devnull,'wet',tape],env)
  results=[]; hist=[]
  for path in [R/'candidate']+[P/f'candidate-{n}' for n in widths]:
   run([path,B/'pack.private',i,tmp,'wet',tape,ev],dict(env,ROOM_TRACE=str(tp)) if trace else env)
   assert tape.read_bytes()==ev.read_bytes()
   wav=tmp.read_bytes(); x=np.frombuffer(wav,'<f4').reshape(-1,2); assert x.shape==(264600,2) and np.isfinite(x).all()
   if path==R/'candidate': assert hashlib.sha256(wav).hexdigest()==old[i]['trial_sha256']
   results.append(x)
   if trace:
    hist.append(np.fromfile(tp,'<f4').reshape(-1,10))
  x=results[0]
  d={}
  for k,y in zip(widths,results[1:]):
   d[k]=dict(words_equal=np.array_equal(x.view('u4'),y.view('u4')),whole=pair(x,y),attack=pair(x[:4410],y[:4410]),release=pair(x[198450:],y[198450:]),tail=pair(x[-44100:],y[-44100:]),bands=bands(x,y))
  if trace:
   for k,u in zip(widths,hist[1:]):
    h=hist[0];assert (h[:,(0,1,2,3,8,9)].view('u4')==u[:,(0,1,2,3,8,9)].view('u4')).all()
    d[k]['pre_room_source_exact']=True
    m=lambda x:np.stack((np.mean(x,axis=1),.5*(x[:,0]-x[:,1])),axis=-1)
    d[k]['mid_side']=pair(m(x[-44100:]),m(results[1][-44100:]))
    d[k]['room_mid_side']=pair(m(h[-44100:,4:6]),m(u[-44100:,4:6]))
    d[k]['room_bus_all']=pair(h[:,4:6],u[:,4:6]);d[k]['room_bus_tail']=pair(h[-44100:,4:6],u[-44100:,4:6]);d[k]['room_bus_bands']=bands(h[:,4:6],u[:,4:6])
 return dict(id=i,label=table['rows'][i]['label'],matched=bool(i in old),results=d)
full='--all' in sys.argv
ids=[1,4,18,22]
with cf.ThreadPoolExecutor(int(os.environ.get("JOBS",6))) as pool:
 for q in pool.map(lambda i:record(i,[169] if full else [169],i in named and not full),range(len(table['rows'])) if full else ids):
  print(json.dumps(q),flush=True)
