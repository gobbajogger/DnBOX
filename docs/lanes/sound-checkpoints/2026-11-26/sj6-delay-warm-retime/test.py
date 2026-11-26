import ctypes as C,json,numpy as n
P=C.c_void_p
F=n.ctypeslib.ndpointer('f4',ndim=1,flags='C_CONTIGUOUS')
H=n.ctypeslib.ndpointer('i2',ndim=1,flags='C_CONTIGUOUS')
def abi(x):
 a=C.CDLL('./'+x+'.so')
 for name,rt,aa in [('new',P,[C.c_uint,C.c_uint]),('free',None,[P]),('info',None,[P,F]),('ring',None,[P,H]),('can',C.c_int,[P,C.c_uint,C.c_float,C.c_uint]),('set',None,[P,C.c_uint,C.c_uint,C.c_uint,C.c_uint,C.c_float]),('process',None,[P,C.c_size_t]+[F]*4)]:
  f=getattr(a,'stock_'+name);f.restype=rt;f.argtypes=aa;setattr(a,name,f)
 return a
B,M=[abi(x) for x in ('base','new')]
def state(a,p):
 z=n.empty(9,'f4');a.info(p,z);return z
def ring(a,p):
 z=n.empty(2048,'i2');a.ring(p,z);return z.reshape(1024,2)
def render(a,p,x,nmax=256):
 y=n.empty_like(x)
 for i in range(0,x.shape[1],nmax):
  s=slice(i,i+nmax);a.process(p,x[:,s].shape[1],*x[:,s],*y[:,s])
 assert n.isfinite(y).all()
 return y
def check(a,p,b,q):
 assert n.array_equal(state(a,p),state(b,q))
 assert n.array_equal(ring(a,p),ring(b,q))
def origin(rate,N):
 t=n.arange(N)/rate
 return n.array([.14*n.sin(2*n.pi*113*t)+.04*n.sin(2*n.pi*291*t),.19*n.sin(2*n.pi*237*t)],'f4')
def get(a,sr,route,t,fb=64,sy=0,bpm=120):
 p=a.new(sr,route);a.set(p,t,fb,127,sy,bpm);return p
rows=[];guards=0
for sr in (44100,48000):
 for raw0,raw1,s,bpm,N in [(21,42,0,120,40017),(5,127,1,120,34003),(32,127,0,120,34027),(21,127,1,12,37009),(38,42,0,120,19033)]:
  for fb in (0,64,127):
   b=get(B,sr,0,raw0,fb);m=get(M,sr,0,raw0,fb)
   x=origin(sr,N)
   assert n.array_equal(render(B,b,x,61),render(M,m,x,127))
   check(B,b,M,m)
   before=state(M,m);old=ring(M,m)
   B.set(b,raw1,fb,127,s,bpm);M.set(m,raw1,fb,127,s,bpm)
   after=state(M,m);stock=state(B,b)
   S,filled,phase,cur,_,summ,_,_,pos=map(float,before)
   D=int(after[0]);ratio=int(D/S)
   guarded=ratio<=filled and before[3]*S>=D and ratio>1
   if guarded:
    assert after[1]==int(filled)//ratio and after[2]==phase and after[3]*D==cur*S and after[5]==summ and after[6]==1250 and after[8]==pos
    assert stock[1]==0 and stock[5]==0 and stock[6]==0
    new=ring(M,m); pos=int(pos)
    for j in range(int(after[1])):
     idx=(pos-1-j*ratio-n.arange(ratio))&1023
     expect=n.trunc(old[idx].sum(axis=0)/ratio).astype('i2')
     assert n.array_equal(new[(pos-1-j)&1023],expect)
   else:check(B,b,M,m);guards+=1
   sil=n.zeros((2,10017),'f4')
   yo=render(B,b,sil,64);yn=render(M,m,sil,73)
   tail=n.max(n.abs(yn[:,:800]))
   assert tail>0 if guarded else n.array_equal(yo,yn)
   A=state(M,m)
   if guarded:
    assert A[6]==0 and A[3]==after[4]
    assert not yo[:,:800].any()
   rows.append(dict(fs=sr,raw0=raw0,raw1=raw1,sync=s,bpm=bpm,feedback=fb,before=before.tolist(),baseline=stock.tolist(),after=after.tolist(),ramp_end=A.tolist(),active=bool(guarded),retained_peak_first800=float(tail)))
   B.free(b);M.free(m)
 # Every actual cold time value, sync guards via real helper, class equivalence.
 for route in (0,1):
  for sync in (0,1):
   for tempo in (.0001,12,120,360):
    for raw in range(128):
     b=get(B,sr,route,raw,sy=sync,bpm=tempo)
     m=get(M,sr,route,raw,sy=sync,bpm=tempo)
     assert B.can(b,raw,tempo,sync)==M.can(m,raw,tempo,sync)
     check(B,b,M,m); B.free(b);M.free(m);guards+=1
 for route,t0,t1,sync,bpm,N in [(1,5,127,1,120,17003),(0,42,5,0,120,19003),(0,0,127,1,12,19003),(0,5,127,1,120,3),(0,42,41,0,120,20003),(0,42,127,1,.0001,20019)]:
  b=get(B,sr,route,t0);m=get(M,sr,route,t0)
  r=origin(sr,N);render(B,b,r,119);render(M,m,r,67)
  B.set(b,t1,64,127,sync,bpm);M.set(m,t1,64,127,sync,bpm)
  check(B,b,M,m)
  r=origin(sr,11027)
  assert n.array_equal(render(B,b,r,61),render(M,m,r,127));check(B,b,M,m)
  B.free(b);M.free(m);guards+=1
print(json.dumps(dict(rows=rows,total=len(rows),retained=sum(r['active'] for r in rows),unchanged_cases=guards),indent=2))
