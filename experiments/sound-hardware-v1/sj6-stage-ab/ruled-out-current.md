# SuperJ6 signed A/B: current rejected explanations

- Master volume, per-preset normalization: not a fix. Retain the original source/controls.
- Brainwave, physical MIDI 36, 44.1 kHz: direct original warmed Main A-only vs Main B-only captures differ only in parameter IDs 1 and 3. Both authored level detents were 1. At six shared frequencies between 65.4 and 392.4 Hz, compare the complex ratio after independent compact exact Main source leaves. Max magnitude discrepancy **0.03327 dB early held, 0.00933 dB late held**; max phase **0.887° / 0.562°**. Repeat floor under 0.000088 dB. Unequal Main balance is not the approx. 12 dB common held deficit in this cue. This ratio cancels shared transfer; the shared absolute gain is not measured at the installed mixer. The exploratory release ratio (within 0.079 dB) does not qualify all envelopes or other patches. The real mixer initializer retains the Apple node; wrapper fields at 1 are not writes to Apple gain/pan. First Main A attach enters platform negotiation without giving us mixer PCM. This does not justify any global gain.
- Simple anchored predictive coding of the authentic full-rate Room histories: independent stereo impulse and changing signed stereo input, actual current original raw kernel. The best Type II impulse result among tested histories was +1.71 dBr signed error with 27,160 B of history; best dense −1.93 dBr with 33,990 B. Even the smallest candidate was 15,520 B; not viable. This is a specific tested predictor, not an impossibility result.
- Slow eight-line Room: 684 B actual PI32 state, better impulse onset 1960 vs original 1981 and former 496. But Type II impulse has 0.00281% energy over 8 kHz vs original 34.99%, and late level is 15.31 dB low; dense is 21.73 dB low. Float32 3900 B on same clock didn't recover the lost behavior. No adoption.
- Sparse full-rate Room (one dominant sample per 256 dry frames, true first reflection indices in the offline original clock, compact independent late feedback): impulse onset exact and early −19.29 dBr signed error, but dense/chord raw residual only −0.013/−0.020 dBr and late return 41.79/42.57 dB too quiet. No adoption even though the impulse looked more promising. No linked firmware cost because the exact early clock was borrowed from the original offline.
- Compact Delay: the symptom measured near −23 dB at 250 ms was the low alias around **243.75 Hz**, not the true magnitude of a 3 kHz output. Stride 16 produces a 2756.25-Hz history clock, which explains the observed fold. Restoring the first 3 kHz repeat in bounded tests is not enough: 6 kHz still lost and long taps/feedback diverge. The separate representable live-retime behavior patch does not change steady bandwidth.
- The alternative source-only proof packages are local bench artifacts (no app binary or PCM committed). Results above do not imply whole-engine or all-preset parity.

## Held Main ratio replay

Requires previously captured original AU signed stems and the signed recovered Main leaf samples; the script pins the original capture and app SHA.

```python
"""Aligned disjoint loaded Main roles vs prepared pre-Apple host leaves."""
from pathlib import Path
import hashlib,json,tarfile,io
import numpy as np
sha=lambda x:hashlib.sha256(x).hexdigest()
p=Path('sj6-mixer-next/private/SJ6-Brainwave-Direct-AUv3-Fanin.tgz')
assert sha(p.read_bytes())=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
g=Path('recovered-sound/main-route/exact-feature/saw-after-auth.private.f32')
host=np.fromfile(g,dtype='<f4').reshape(3,148684)[:2]
refs=[]
with tarfile.open(p) as t:
 for n in ('osc1-only','osc2-only'):
  re=[]
  for state in ('','-repeat'):
   f='native-warm-brainwave-fanin-authored-'+n+state+'/'
   m=json.load(t.extractfile(f+'capture.json'))
   a=np.load(io.BytesIO(t.extractfile(f+'output.npy').read()),allow_pickle=False)
   assert m['pcm_sha256']==sha(a.tobytes()) and m['parameter_readback_after']==m['parameters']
   assert m['rate']==44100 and m['warm_frames']==16384 and a.shape==(2,148684)
   assert m['installed_au_sha256']=='1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257'
   re.append((m,a))
  refs.append(re)
for (r,a),(s,b) in zip(*refs):
 assert (r['schedule'],r['buffers'])==(s['schedule'],s['buffers'])
 wd=lambda m:{int(x['address']):x['value'] for x in m['parameters']}
 assert {p for p in wd(r) if wd(r)[p]!=wd(s)[p]}=={1,3}
def eval(f,l,r,a,b):
 w=np.hanning(r-l)*np.exp(-2j*np.pi*np.arange(l,r)*f/44100.)
 z=lambda x:np.asarray(x[...,l:r]@w)
 return z(b)/z(a)
def met(z):
 return {'db':np.asarray(20*np.log10(abs(z))).tolist(),'deg':np.asarray(np.angle(z,deg=True)).tolist()}
f0=440*2**((36-69)/12)
rows={}
for n,l,r in [('early',20480,28672),('late',28672,37634),('release',38434,42000)]:
 cuts={}
 for m in (1,2,3,4,5,6):
  freq=f0*m
  natives=[eval(freq,l,r,refs[0][j][1],refs[1][j][1]) for j in (0,1)]
  proxy=eval(freq,l,r,*host)
  cuts[m]={'hz':freq,'native_Osc2_over_Osc1':[met(z) for z in natives],'host_Osc2_over_Osc1':met(proxy),
           'ratio_gap':met(proxy/natives[0]),'repeat_floor':met(natives[1]/natives[0])}
 rows[n]=cuts
print(json.dumps(dict(reference=sha(p.read_bytes()),host=sha(g.read_bytes()),scopes='Fixed shared-frequency pilots, no aligning or fitted scale. Native stems terminal; host postvoice/Main filter conditional, same warmed note schedule and declared controls. A shared settled linear suffix cancels at same frequency up to finite window/envelope; release ratios are indicative only.',case=rows),indent=2))
```

```python
import numpy as np, hashlib, sys, json
from pathlib import Path
import emu,macho
from unicorn.arm64_const import UC_CPU_ARM64_MAX
from unicorn import UC_HOOK_CODE
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
old=emu.Uc
def mk(*a,**k):u=old(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=mk
P=Path(sys.argv[1]); n=16384
A={'_sp_revsc_create':0x1003b3398,'_sp_revsc_init':0x1003b33c4,'_sp_revsc_compute':0x1003b3640,'_sp_revsc_destroy':0x1003b360c}
def original(x, cut, bits, anchor):
 e=emu.Emu(str(P/'mac-arm64')); assert all(e.im.sym_by_name[k]==v for k,v in A.items())
 sp,ind,f=e.calloc(256),e.calloc(8),e.calloc(16)
 e.w32(sp+8,44100);e.w32(sp+12,2)
 assert e.call(A['_sp_revsc_create'],ind)==1
 s=e.r64(ind); assert e.call(A['_sp_revsc_init'],sp,s)==1
 e.wf(s,.7779527902603149);e.wf(s+4,cut);assert e.r64(s+0x1a8)==98760
 y=np.empty_like(x); meta=[None]*8; peaks=[0]*8; minages=[1<<31]*8; rings=np.zeros((2,8192),'f4')
 sample=0
 pred=[0.]*8
 steps=[.00025]*8
 clips=[0]*8
 def record(*_):
  lane=e.x(22); state=e.x(8); w=e.x(9)&0xffffffff; cap=e.r32(state+0x2c); base=e.x(11)
  assert state==s+0x30*lane and lane<8 and w<cap and cap<8192,(hex(state),hex(s),lane,w,cap)
  meta[lane]=(w,cap,base)
 def feeds(*_):
  lane=e.x(22);write,cap,base=meta[lane]
  taps=[((e.x(r)-base)//4)%cap for r in (14,13,12,10)]
  assert all(base<=e.x(r)<base+cap*4 for r in (14,13,12,10))
  ages=[(write-index)%cap for index in taps]
  peaks[lane]=max(peaks[lane],max(ages));minages[lane]=min(minages[lane],min(ages))
  if bits:
   # Write is already complete, but no read tap aliases today's write.
   assert min(ages)>0
   addr=base+4*write
   exact=e.rf(addr)
   if sample%anchor==0:
    # Model an exact Float32 anchor, including decoder step reset.
    pred[lane]=exact
    steps[lane]=.00025
   else:
    step=steps[lane]
    q=round((exact-pred[lane])/step)
    high=2**(bits-1)-1
    clips[lane]+=not -high-1<=q<=high
    q=min(high,max(-high-1,q))
    pred[lane]=float(np.float32(pred[lane]+q*step))
    # The update depends only on the symbol and previous decoded state.
    steps[lane]=max(.000002,min(.125,step*(1.5 if abs(q)>=high else (.85 if abs(q)*2<high else 1.))))
    e.wf(addr,pred[lane])
 e.uc.hook_add(UC_HOOK_CODE,record,begin=0x1003b377c,end=0x1003b377c)
 e.uc.hook_add(UC_HOOK_CODE,feeds,begin=0x1003b3884,end=0x1003b3884)
 for sample in range(n):
  rings[:,sample%8192]=x[:,sample]
  e.wf(f,float(x[0,sample]));e.wf(f+4,float(x[1,sample]))
  assert e.call(A['_sp_revsc_compute'],sp,s,f,f+4,f+8,f+12)==1
  y[:,sample]=e.rf(f+8),e.rf(f+12)
 e.call(A['_sp_revsc_destroy'],ind);assert not e.unknown,e.unknown
 return y,dict(max_ages=peaks,min_ages=minages,caps=[a[1] for a in meta],clipped=clips)
def digest(y):return hashlib.sha256(y.astype('<f4').tobytes()).hexdigest()
def first(y):return int(np.flatnonzero(np.any(y,axis=0))[0])
x=np.zeros((2,n),'f4');x[0,0]=.25;x[1,37]=-.125
z=np.arange(n,dtype=np.int64); dense=np.stack((((z*3023%65536)-32768)/262144.,((z*1597%65536)-32768)/262144.)).astype('f4')
rows=[]
for name,i in [('impulse',x),('dense',dense)]:
 for cut in (4000.,20000.):
  r,rs=original(i,cut,0,16)
  for bits,anchor in [(4,8),(4,32),(6,8),(8,8),(8,32)]:
   y,ys=original(i,cut,bits,anchor)
   def err(a,b):
    return float(20*np.log10(np.linalg.norm((a-b).astype('f8'))/np.linalg.norm(b.astype('f8'))))
   rows.append(dict(cue=name,cut=cut,bits=bits,anchor=anchor,reference_first=first(r),candidate_first=first(y),
      signed_error_dBr=err(y,r),early_error_dBr=err(y[:,:4096],r[:,:4096]),tail_error_dBr=err(y[:,4096:],r[:,4096:]),
      lower_bound_history_bytes=sum(((c+anchor-1)//anchor)*(4+((anchor-1)*bits+7)//8) for c in ys['caps']),
      clipped=sum(ys['clipped']),ref_digest=digest(r),candidate_digest=digest(y)))
   print(json.dumps(rows[-1]),flush=True)
```

## Sub versus Main ratio

Independent original Main-pair-only/Sub-only Brainwave arms at 44.1 kHz use the same clock and readbacks except source faders 1/3/202. Three shared settled harmonics of 32.703 Hz: conditional compact Main −11.805/−11.848/−11.928 dB; Sub −12.344/−12.385/−12.468 dB. Relative level gap −0.540/−0.537/−0.540 dB, but phases 2.655°/8.034°/13.426° grow with harmonic and over time. Supports a shared magnitude deficit for this cue, not a single-gain explanation. Conditional Main HP was original, Sub host; no internal installed tap measured.

```python
"""Loaded Main-pair and Sub stems vs same-cue staged reference; no rescaling."""
from pathlib import Path
p=Path('sj6-newer-level-boundary/check.py')
s=p.read_text().split('print(json.dumps')[0].replace(' def db(n,d):',' return rc,real,z\n def db(n,d):')
exec(compile(s,str(p),'exec'))
main,sub=report('main'),report('sub')
a,x,u=main;b,y,v=sub
assert a['schedule']==b['schedule'] and a['buffers']==b['buffers']
word=lambda q:{int(z['address']):z['value'] for z in q['parameters']}
aa,bb=map(word,(a,b))
assert {k for k in aa if aa[k]!=bb[k]}=={1,3,202}
assert all(aa[k]==bb[k]==0 for k in (11,17,40,41,107,201))
def ratio(z,w):
 return 20*np.log10(np.maximum(abs(z),1e-30)/np.maximum(abs(w),1e-30))
def score(l,r):
 rows={}
 for i in range(1,9):
  hz=440*2**((24-69)/12)*i
  window=np.hanning(r-l)*np.exp(-2j*np.pi*np.arange(l,r)*hz/44100.)
  # Report actual magnitude of each role here so near-zero Main odd bins
  # cannot be mistaken for physically matched shared-frequency references.
  X,Y,U,V=[z[:,l:r]@window / np.sum(abs(window)) for z in (x,y,u,v)]
  witness=(abs(X)>.003)&(abs(Y)>.003)
  rows[i]=dict(hz=hz,admitted=witness.tolist(),
   native_main_mag=abs(X).tolist(),native_sub_mag=abs(Y).tolist(),
   conditional_main_db=ratio(U,X).tolist(),conditional_sub_db=ratio(V,Y).tolist(),
   residual_role_ratio_db=(ratio(V,Y)-ratio(U,X)).tolist(),
   residual_role_ratio_phase_deg=np.angle((V/Y)/(U/X),deg=True).tolist())
 return rows
out={w:score(l,r) for w,l,r in [('early',20480,28672),('late',28672,37634),('release',38434,42000)]}
print(json.dumps(dict(scope='same-cue direct native dry Main-pair and Sub isolation; model Main and Sub share physical output suffix but differ upstream Apple-vs-host HP; no identical internal stage claim, no audio fit or rescale. Shared frequency screen is applied only where both native pilot phasors exceed .003.',native_hashes=[a['pcm_sha256'],b['pcm_sha256']],cases=out),indent=2))
```
