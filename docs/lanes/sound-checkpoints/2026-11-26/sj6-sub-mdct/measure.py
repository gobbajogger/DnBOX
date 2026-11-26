from pathlib import Path
import ctypes as c, hashlib, io, json, math, re, sys, tarfile, zipfile,struct
import numpy as np
o,b=map(Path,sys.argv[1:])
# Reuse authorized matching sample capsule decoder, no compiled app import.
s=Path('recovered-sound/sj6-sub-handoff/probe.py').read_text().split('ipa=')[1].split('nat=Path(')[0]
exec(compile('ipa='+s,'existing private SFZ container decoder','exec'))
sha=lambda z:hashlib.sha256(z).hexdigest()
assert sha(data)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
from scipy.fft import dct
import zlib
# Original source buffer never exported. Finite fixed menu before note scores.
Nmenu=(512,1024,2048)
Kmenu=(2,4,8,12,16,24,32,48,64)
def forward(x,N):
    x=np.pad(x,(N,N+(-len(x))%N))
    basis=np.lib.stride_tricks.sliding_window_view(x,2*N)[::N].copy()
    win=np.sin((np.arange(2*N)+.5)*np.pi/(2*N))
    f=basis*win;H=N//2
    return dct(np.concatenate((-f[:,N:3*H][:,::-1]-f[:,3*H:],f[:,:H]-f[:,H:N][:,::-1]),axis=1),type=4,norm='ortho',axis=1),win
def backward(c,win,n):
    N=c.shape[1];H=N//2
    u=dct(c,type=4,norm='ortho',axis=1)
    blocks=np.concatenate((u[:,H:],-u[:,H:][:,::-1],-u[:,:H][:,::-1],-u[:,:H]),axis=1)*win
    y=np.zeros((len(c)+1)*N)
    for j,v in enumerate(blocks):y[j*N:(j+2)*N]+=v
    return y[N:N+n]
def rank(x,y):
    rx,ry=np.linalg.norm(x),np.linalg.norm(y);err=np.linalg.norm(x-y)
    return [float(20*np.log10(max(err/ry,1e-20))),float(20*np.log10(max(rx/ry,1e-20))),float(x@y/max(rx*ry,1e-30))]
def score(x,y,window):return {k:rank(x[l:r],y[l:r]) for k,(l,r) in window.items()}
def count_encode(ix,q):
    # int16 quantized global lattice, absolute coefficient bin plus signed value,
    # 16 hop standalone chunks with length+CRC, 32bit restart index per chunk.
    # Upper bound to implementable, measured exact independent zlib streams
    # with fixed binary scale globally and packed 16-bit index/value pairs.
    widths=np.dtype([('index','<u2'),('level','<i2')])
    m=np.empty(ix.shape,dtype=widths);m['index']=ix;m['level']=q
    total=0
    for i in range(0,len(m),16):
      raw=m[i:i+16].tobytes()
      payload=zlib.compress(raw,9)
      assert zlib.decompress(payload)==raw
      total+=min(len(payload),len(raw))+10
    return total
row={}
floor=-40.
for g in regions:
  name=g['sample'].replace('\\','/').split('/')[-1]
  sr,a=pack[name];assert sr==44100
  x=a.astype('f8')/32768.;lo,hi,root=[int(g[k]) for k in ('loop_start','loop_end','pitch_keycenter')]
  guard=round(10*sr/(220*2**((root-69)/12)))
  cuts={'strike-0-50ms':(0,sr//20),'early-body-100-500ms':(sr//10,sr//2),
       'blind-loop-head':(lo+guard,lo+2*guard),'blind-loop-tail':(hi-2*guard,hi-guard),
       'actual-loop-boundary-before':(hi-256,hi),'actual-loop-boundary-after':(lo,lo+256),'full-zone':(0,len(x))}
  row[root]={}
  for N in Nmenu:
    c,w=forward(x,N);roundtrip=backward(c,w,len(x))
    assert np.max(abs(x-roundtrip))<1.e-12
    # Same nonzero quantizer for complete bank. No rebalance by root/note.
    # f64 to signed fixed point step 2^-9, sufficient headroom for 2048.
    Q=512.
    order=np.argsort(-abs(c),axis=1,kind='stable')
    for k in Kmenu:
      ix=np.sort(order[:,:k],axis=1)
      z=np.rint(np.take_along_axis(c,ix,axis=1)*Q)
      assert np.max(abs(z))<=32767
      q=z.astype('i2')
      a=np.zeros_like(c);np.put_along_axis(a,ix,q.astype('f8')/Q,axis=1)
      n=count_encode(ix,q);y=backward(a,w,len(x))
      v=score(y,x,cuts);row[root][f'{N}:{k}']=dict(B=n,windows=v,worst=max(z[0] for z in v.values()))
def opt(limit):
  winner={}
  for root,arms in row.items():
    keep=[(v['B'],k) for k,v in arms.items() if v['worst']<limit]
    if keep:winner[root]=min(keep)[1]
  return dict(limit=limit, selected=winner, complete_B=288+sum(row[z][k]['B'] for z,k in winner.items()) if len(winner)==len(row) else None)
curves={}
for k in next(iter(row.values())):
 r=[v[k] for v in row.values()]
 curves[k]=dict(B=288+sum(z['B'] for z in r),worst_dbr=max(z['worst'] for z in r))
print(json.dumps(dict(scope='private lapped transform, no oscillator model, no generation, alignment, gain or exported components',
  exact_full_coeff_error_lt='1e-12 maximum', source=sha(data), pins='36b39d9d six source windows plus full-zone aggregate; no Raw holdout or fit claim',
 adaptive=[opt(x) for x in (0,-20,-30,-40)],uniform=curves,zones=row,
 pool_diagnostic_gross_B=112*1024, five_BREAKs_diagnostic_before_banks_and_reserve_B=8192,
 downside='512..2048 sample hop plus two overlapping windows, reference host FFT/zlib only; no target decode or random I/O acceptance'),indent=2,allow_nan=False))
