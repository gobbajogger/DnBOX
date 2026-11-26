import ctypes as c, hashlib,json,math,os
from pathlib import Path
import numpy as np
D=Path(__file__).resolve().parent
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
old=c.CDLL(str(D/'original.so'));base=c.CDLL(str(D/'compact.so')); cand=c.CDLL(str(D/'heterodyne.so'))
ptr=c.c_void_p
old.sj6_new.argtypes=[c.c_double];old.sj6_new.restype=ptr
old.sj6_params.argtypes=[ptr,c.c_double,c.c_float,c.c_float,c.c_int]
old.sj6_process.argtypes=[ptr,c.c_size_t]+[fp]*4;old.sj6_free.argtypes=[ptr]
cand.heterodyne_new.argtypes=[c.c_uint,c.c_float];cand.heterodyne_new.restype=ptr
cand.heterodyne_params.argtypes=[ptr,c.c_float,c.c_float,c.c_float,c.c_uint];cand.heterodyne_process.argtypes=[ptr,c.c_size_t]+[fp]*4;cand.heterodyne_free.argtypes=[ptr]
base.compact.argtypes=[c.c_uint]*3+[fp,fp,c.c_uint]
sr=44100
def tm(raw):return np.float32(1.5/127.)*raw
def match(x,raw,fbn,ping=0,cutoff=520,parts=None):
 t=float(tm(raw)); fb=float(np.float32(.99/127.)*fbn)
 result=[]
 for isold in (0,1):
  out=np.empty_like(x)
  if isold:
   p=old.sj6_new(sr);old.sj6_params(p,t,fb,.1,ping);pr=old.sj6_process;free=old.sj6_free
  else:
   p=cand.heterodyne_new(sr,cutoff);assert cand.heterodyne_params(p,t,fb,.1,ping)==1;pr=cand.heterodyne_process;free=cand.heterodyne_free
  if parts:
   at=0; i=0
   while at<x.shape[1]:
    n=min(parts[i%len(parts)],x.shape[1]-at);sl=slice(at,at+n)
    pr(p,n,x[0,sl],x[1,sl],out[0,sl],out[1,sl]);at+=n;i+=1
  else:pr(p,x.shape[1],*x,*out)
  free(p);assert np.isfinite(out).all();result.append(out)
 if ping:return result[1],result[0],None
 y=x.copy();base.compact(raw,fbn,127,*y,y.shape[1])
 return result[1],result[0],y
def db(v):return float(20*np.log10(max(float(v),1e-15)))
def level(y,x):return db(np.linalg.norm(y)/max(np.linalg.norm(x),1e-20))
def carrier(x,hz,win):
 ix=np.arange(len(x));p=np.exp(-2j*np.pi*hz*ix/sr)
 return abs(p@(x*win))
tone=[]
for raw in (21,42):
 delay=round(float(tm(raw))*sr)
 for f in (220,700,1000,2600,3000,3400,6000):
  n=delay*5; t=4096; x=np.zeros((2,n),np.float32);x[0,:t]=.32*np.hanning(t)*np.sin(2*np.pi*f/sr*np.arange(t))
  for fb in (0,64,106):
   for cut in (350,520,600):
    ref,y,b=match(x,raw,fb,cutoff=cut)
    assert not y[1].any()
    for rep in ((1,) if fb==0 else (1,2,4)):
     q=slice(rep*delay-128,rep*delay+t+128); o=ref[0,q];w=np.hanning(len(o)); reference=carrier(o,f,w)
     if reference<1e-5:continue
     u={'raw':raw,'time_ms':1000*float(tm(raw)),'f':f,'cutoff':cut,'fb_raw':fb,'repeat':rep}
     for k,v in [('recursive',y[0,q]),('compact',b[0,q])]:
      ft=np.fft.rfftfreq(16384,d=1/sr); z=np.abs(np.fft.rfft(v*w,16384));z[abs(ft-f)<160]=0
      u[k]=dict(carrier_db=db(carrier(v,f,w)/reference),highspur_db=db(z.max()/reference),spur_hz=float(ft[z.argmax()]),wave_residual_dbr=level(v-o,o))
     tone.append(u)
imp=[]
for raw in (21,42):
 d=round(float(tm(raw))*sr)
 for phase in (0,5,13,21,32):
  x=np.zeros((2,3*d),np.float32);x[0,phase]=.5
  for cut in (350,520,600):
   ref,y,b=match(x,raw,64,cutoff=cut)
   ix=np.arange(d-300,d+512)
   out=[]
   for v in (ref,y,b):
    qq=np.asarray(v[0,ix],dtype=float)**2
    out.append(dict(peak_offset=int(ix[qq.argmax()])-d-phase,energy_centroid=float(qq@ix/(qq.sum() or 1))-d-phase))
   imp.append(dict(raw=raw,phase=phase,cutoff=cut,ref=out[0],recursive=out[1],compact=out[2]))
cuebytes=(D/'cue.f32').read_bytes();assert hashlib.sha256(cuebytes).hexdigest()=='bcfce38842c85e3e1aaf704824693a042ff513c68e824c1af2ecc189e1c88e9a'
cue=np.frombuffer(cuebytes,'<f4').copy().reshape(2,-1)
x=np.pad(cue,((0,0),(0,88200)))
music=[]
def bands(x):
 n=len(x[0]); f=np.fft.rfftfreq(n,1/sr)
 z=np.fft.rfft(np.hanning(n)[None,:]*x,axis=1)
 return [z[:,(f>=a)&(f<b)] for a,b in ((30,600),(600,2400),(2600,3450),(3450,16000))]
for raw in (21,42):
 for fb in (64,106):
  for ping in (0,1):
   for cut in (350,520,600):
    ref,y,b=match(x,raw,fb,ping,cut)
    _,chunked,_=match(x,raw,fb,ping,cut,(17,128,67,256))
    assert np.array_equal(y.view('u4'),chunked.view('u4'))
    # Mark synthetic isolated FX experiment; cue is actual recorded Brainwave renderer preceding these effects.
    a={'raw':raw,'fb_raw':fb,'ping':ping,'cutoff':cut}
    for k,z in [('recursive',y),('compact',b)]:
     if z is None:continue
     s=slice(cue.shape[1],cue.shape[1]+88200)
     orig=ref[:,s];p=z[:,s];b1=bands(orig);b2=bands(p)
     a[k]={'tail_level_delta_dB':level(p,orig),'tail_residual_dBr':level(p-orig,orig),'tail_band_level_dB':[level(q,v) for v,q in zip(b1,b2)],'tail_band_residual_dBr':[level(q-v,v) for v,q in zip(b1,b2)]}
    music.append(a)
s=cand.heterodyne_new(sr,520)
bound={}
for sec in (0.,.001,.248,.496,.5,.503,.6,1.5,16):bound[str(sec)]=bool(cand.heterodyne_params(s,sec,.499,.1,0))
cand.heterodyne_free(s)
print(json.dumps(dict(host_state_bytes=cand.heterodyne_size(),preserved_midpoint_region='3k complex carrier plus independent DC band; no EQ/gain-matching',cue_source_sha256=hashlib.sha256(cuebytes).hexdigest(),guard=bound,tone=tone,impulse=imp,real_cue=music),indent=2))
