# Type II/Type I physical first-return projection

At pinned current macOS Build 9 ARM64 SHA 6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850, raw Costello matched signed dry source begins 1,981 frames after the onset of this asymmetric impulse pair. The compact Type II begins at 496, an advance of 1,485 samples (33.67 ms) under the exact same inlet. This comparison is at the raw wet leaf and does not include on-device audio routing or original Apple HP, output amplifier or FX.

The runtime projection hooks the authentic circular write-index instruction .3b377c to witness lane, actual write index, ring capacity and base. Before cubic interpolation at .3b3884 it computes age for all four actual addresses, then substitutes operands from one shared L and R ingress history. No timing, per-line read positions, RNG, modulation, interpolation, per-line damping, mix equations or original executable is rewritten. The original reference runs the same hook for address and length census but gets no audio operand substitution. The model is feed-forward at the substituted boundaries and intentionally discards recirculated samples at the read side; it is not an exact late network. Comparison is integer view of Float32 signed words, no normalization or alignment.

On both physical 4,000 and 20,000 Hz damping, exact reference vs projection matches through frame 3,925 on the impulse (7,852 stereo sample words through 89.025 ms from tick zero). Genuine independent dense stereo stimulus matches through 3,888 (7,778 words; 88.186 ms). First differences coincide with the earliest actually audible recirculated material: 3,926 / 3,889. Calculated max age across the first 12,288 frames grouped by input channel yields measured working history ceilings of 3,937 L + 4,163 R cells. The current compact ring has 350 signed-16 cells / 700 B. The projected full-rate direct raw representation for this broader frame window takes 32,400 B Float32, or would take 8,100 B at unverified 8-bit, versus 98,760 B for the eight real Float32 rings. This is the measured capacity of the chosen two-ring representation, not an information-theoretic lower bound or a global modulation bound. The projection still uses original arithmetic and is not itself a native implementation. Limiting to the first return alone won't magically share storage with an active Delay: the room input is downstream, not its existing ring payload.

No evidence for a faithful fixed compact short onset+tail splice or enough firmware RAM; such a splice cuts original feedback starting at an observed finite nonzero time. No gate test, controls, master gain, patch tuning, release/change to product, or app-adoption claim.

Run early.py with the existing local ephemeral pin and prep instructions in README.md. Signed digests and read positions only in early.json; no audio assets.

## Diagnostic source

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
P=Path(sys.argv[1]); n=12288
A={'_sp_revsc_create':0x1003b3398,'_sp_revsc_init':0x1003b33c4,'_sp_revsc_compute':0x1003b3640,'_sp_revsc_destroy':0x1003b360c}
def original(x, cut, projected):
 e=emu.Emu(str(P/'mac-arm64')); assert all(e.im.sym_by_name[k]==v for k,v in A.items())
 sp,ind,f=e.calloc(256),e.calloc(8),e.calloc(16)
 e.w32(sp+8,44100);e.w32(sp+12,2)
 assert e.call(A['_sp_revsc_create'],ind)==1
 s=e.r64(ind); assert e.call(A['_sp_revsc_init'],sp,s)==1
 e.wf(s,.7779527902603149);e.wf(s+4,cut);assert e.r64(s+0x1a8)==98760
 y=np.empty_like(x); meta=[None]*8; peaks=[0]*8; minages=[1<<31]*8; rings=np.zeros((2,8192),'f4')
 sample=0
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
  if projected:
   for age,r in zip(ages,(5,6,7,16)):
    e.sets(r, float(rings[lane&1,(sample-age)%8192]) if age<=sample else 0.)
 e.uc.hook_add(UC_HOOK_CODE,record,begin=0x1003b377c,end=0x1003b377c)
 e.uc.hook_add(UC_HOOK_CODE,feeds,begin=0x1003b3884,end=0x1003b3884)
 for sample in range(n):
  rings[:,sample%8192]=x[:,sample]
  e.wf(f,float(x[0,sample]));e.wf(f+4,float(x[1,sample]))
  assert e.call(A['_sp_revsc_compute'],sp,s,f,f+4,f+8,f+12)==1
  y[:,sample]=e.rf(f+8),e.rf(f+12)
 e.call(A['_sp_revsc_destroy'],ind);assert not e.unknown,e.unknown
 return y,dict(max_ages=peaks,min_ages=minages,caps=[a[1] for a in meta])
def digest(y):return hashlib.sha256(y.astype('<f4').tobytes()).hexdigest()
def first(y):return int(np.flatnonzero(np.any(y,axis=0))[0])
x=np.zeros((2,n),'f4');x[0,0]=.25;x[1,37]=-.125
z=np.arange(n,dtype=np.int64); dense=np.stack((((z*3023%65536)-32768)/262144.,((z*1597%65536)-32768)/262144.)).astype('f4')
rows=[]
for name,i in [('impulse',x),('dense',dense)]:
 for cut in (4000.,20000.):
  r,rs=original(i,cut,False);p,ps=original(i,cut,True);assert rs==ps
  a=first(r); b=first(r.view('u4')!=p.view('u4'))
  roof=[max(rs['max_ages'][ch::2])+1 for ch in (0,1)]
  assert digest(r[:,:b])==digest(p[:,:b])
  rows.append(dict(cue=name,cut=cut,first_return=a,first_recirculation_difference=b,prefix_sample_words_exact=2*b,prefix_digest=digest(r[:,:b]),prefix_duration_ms=1000*b/44100,
       stock_first_type_ii=496 if name=='impulse' and cut==20000 else None,physical_max_live_history_by_channel=roof,float32_history_bytes=4*sum(roof),signed8_history_bytes=sum(roof),**rs))
print(json.dumps(rows,indent=2))
```

## Result

```json
[
  {
    "cue": "impulse",
    "cut": 4000.0,
    "first_return": 1981,
    "first_recirculation_difference": 3926,
    "prefix_sample_words_exact": 7852,
    "prefix_digest": "b58b453221a69c8f6d0661c62fb8adb2ac6e62631b4916a5c745230fe0021859",
    "prefix_duration_ms": 89.02494331065759,
    "stock_first_type_ii": null,
    "physical_max_live_history_by_channel": [
      3937,
      4163
    ],
    "float32_history_bytes": 32400,
    "signed8_history_bytes": 8100,
    "max_ages": [
      2477,
      2812,
      3271,
      3566,
      3936,
      4162,
      2212,
      1949
    ],
    "min_ages": [
      2451,
      2786,
      3234,
      3537,
      3896,
      4121,
      2200,
      1943
    ],
    "caps": [
      2539,
      2838,
      3317,
      3603,
      3973,
      4198,
      2243,
      1979
    ]
  },
  {
    "cue": "impulse",
    "cut": 20000.0,
    "first_return": 1981,
    "first_recirculation_difference": 3926,
    "prefix_sample_words_exact": 7852,
    "prefix_digest": "e05ef31597f9a53f9bf69550122bd136b38531054addc47b2e97ce4bd4ac692b",
    "prefix_duration_ms": 89.02494331065759,
    "stock_first_type_ii": 496,
    "physical_max_live_history_by_channel": [
      3937,
      4163
    ],
    "float32_history_bytes": 32400,
    "signed8_history_bytes": 8100,
    "max_ages": [
      2477,
      2812,
      3271,
      3566,
      3936,
      4162,
      2212,
      1949
    ],
    "min_ages": [
      2451,
      2786,
      3234,
      3537,
      3896,
      4121,
      2200,
      1943
    ],
    "caps": [
      2539,
      2838,
      3317,
      3603,
      3973,
      4198,
      2243,
      1979
    ]
  },
  {
    "cue": "dense",
    "cut": 4000.0,
    "first_return": 1944,
    "first_recirculation_difference": 3889,
    "prefix_sample_words_exact": 7778,
    "prefix_digest": "78416abfb0100d0e9006900769cfbf1989b958a9c4ff479d402f0c9a02d62b1d",
    "prefix_duration_ms": 88.1859410430839,
    "stock_first_type_ii": null,
    "physical_max_live_history_by_channel": [
      3937,
      4163
    ],
    "float32_history_bytes": 32400,
    "signed8_history_bytes": 8100,
    "max_ages": [
      2477,
      2812,
      3271,
      3566,
      3936,
      4162,
      2212,
      1949
    ],
    "min_ages": [
      2451,
      2786,
      3234,
      3537,
      3896,
      4121,
      2200,
      1943
    ],
    "caps": [
      2539,
      2838,
      3317,
      3603,
      3973,
      4198,
      2243,
      1979
    ]
  },
  {
    "cue": "dense",
    "cut": 20000.0,
    "first_return": 1944,
    "first_recirculation_difference": 3889,
    "prefix_sample_words_exact": 7778,
    "prefix_digest": "aec248441aa38c4bc8e5a287e58d645441ec015b088731eb67fbfdc7dd86bafa",
    "prefix_duration_ms": 88.1859410430839,
    "stock_first_type_ii": null,
    "physical_max_live_history_by_channel": [
      3937,
      4163
    ],
    "float32_history_bytes": 32400,
    "signed8_history_bytes": 8100,
    "max_ages": [
      2477,
      2812,
      3271,
      3566,
      3936,
      4162,
      2212,
      1949
    ],
    "min_ages": [
      2451,
      2786,
      3234,
      3537,
      3896,
      4121,
      2200,
      1943
    ],
    "caps": [
      2539,
      2838,
      3317,
      3603,
      3973,
      4198,
      2243,
      1979
    ]
  }
]
```
