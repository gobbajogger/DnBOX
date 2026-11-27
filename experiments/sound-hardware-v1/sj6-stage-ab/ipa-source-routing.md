# Trace SuperJ6 dry routing and Room directly from the authorized IPA

The iOS AU Mach-O is a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1. The newer current AU is 6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850. Original images, presets and PCM are not included.

## What went wrong in DnBOX

The /6 and center .5 both exist in the November 18 frozen candidate. Its independent review explicitly uses /6 as a finite headroom bound for six voice slots, not a traced original mixer operation. On identical signed input from the original stereo Main receivers that reducer yields −21.58 dB per centered output. Both original versions request the same oscillator mixer trim in authored Brainwave Normal: 0.8670601844787598 (−1.24 dB), with no pan setter on this control path. The current app sender is executed from the actual compiled tail with scoped owner scaffolding; the IPA separately drives the reached resonance property path. Neither sender is master volume or Apple rendered PCM.

The new host bridge accepts independent stereo streams at unity input volume, centered; it rejects all other configurations. Sum channels then apply the reached source trim once. Apple documents the default input bus volume as 1.0 (https://developer.apple.com/documentation/avfaudio/avaudiomixing/volume). Against this explicit source-connected model, the compact reducer is −20.3446 dB in all three same-input probes. New C reproduces the signed model exactly, 18,432 words per arm and combined peak .6326 in this two-source cue. This is not the terminal Brainwave A/B; do not interpret it as a +20 dB boost. Existing compact mono reducer follows a different upstream projection. Match the signal flow, not global gain. The modeled Apple fan-in remains a model rather than an observed internal tap.

## IPA raw Room and filter cross-check

The IPA extension's actual sp_revsc receiver executes via symbols in that image. In four identical signed impulse/dense/held-release Type II and Type I fixtures, every output digest and arrival matches the newer binary exactly. This validates comparing the same raw kernel behavior across the builds, not the complete effect chain. Ran the IPA Normal coefficient and physical receiver oracles separately. Existing Delay clock/cap source work independently used both versions.

## Reproduce

Use the authorized local IPA from project sources. Extract its AU extension privately as au.private and validate the SHA above. Run sj6-old-convergence/probe.py and sj6-old-premaster/sender.py with SJ6_BIN on that image under the project's emu/macho/Unicorn path. Run ipa-room.py with a directory containing that image as mac-arm64. Build source-mix.c as a host shared object with -O2 -ffp-contract=off and run mix-driver.py using the authentic current receiver/source inputs. Raw audio remains local. No Mac required for this diagnostic; no production changes.

### source-mix.c

```c
/* Host diagnostic for the source-proven centered stereo fan-in route.
 * Only call after the four independent signed stereo sampler buffers exist.
 * Do not replace DnBOX's combined mono oscillator reducer with this bridge. */
#include <stddef.h>
typedef struct {const float *left,*right;} stereo_bus;
int sj6_source_mix(unsigned count,const stereo_bus *in,const float *input_volume,
                   const float *input_pan,float original_trim,float *left,float *right)
{
    if(!(original_trim>=0.f&&original_trim<=1.f))return 0;
    for(unsigned b=0;b<4;b++)
        if(input_pan[b]!=0.f||input_volume[b]!=1.f||!in[b].left||!in[b].right)return 0;
    for(unsigned i=0;i<count;i++){
        float l=in[0].left[i],r=in[0].right[i];
        for(unsigned b=1;b<4;b++){l+=in[b].left[i];r+=in[b].right[i];}
        left[i]=l*original_trim;right[i]=r*original_trim;
    }
    return 1;
}
```

### mix-driver.py

```python
"""Run exact pinned compact reduction on genuine current receiver's signed output (private in RAM)."""
from pathlib import Path
import ctypes as C, subprocess, tempfile
p0=Path('sj6-mix-boundary-recheck/wrapper.py')
exec(compile(p0.read_text().split('def run(')[0],str(p0),'exec'))
donor=Path('sj6-fold32-recovery/exact-donor/firmware/src/dsp/dnbx_core.c')
sha=hashlib.sha256(donor.read_bytes()).hexdigest()
assert sha=='70d0aa0ea1cf2192e50cd51b45dffe01533cde368c0a1778959d39312b0ce759'
src=donor.read_text()
tail=src[src.index('        left[k]=0; right[k]=0;'):].split('\n    }',1)[0]
with tempfile.TemporaryDirectory() as t:
 q=Path(t); (q/'r.c').write_text('typedef struct {float gain,pan;} dnbx_layer_params;\n#define DNBX_LAYERS 2\nvoid reduce(float *mix,float *left,float *right, float g,float pan){struct {struct {dnbx_layer_params params;} layer[2];} a={{{{g,pan}},{{g,pan}}}}; const unsigned k=0; __typeof__(a)*s=&a;'+tail+'\n}')
 subprocess.run(['cc','-O2','-ffp-contract=off','-shared','-fPIC',str(q/'r.c'),'-o',str(q/'r.so')],check=True)
 fn=C.CDLL(str(q/'r.so')).reduce
 F=C.c_float; P=C.POINTER(F);fn.argtypes=[P,P,P,F,F];fn.restype=None
 arms=[]
 for role in ['oscSampler1','oscSampler2']:
  d=sampler_owner(role);w=wrapped(d)
  for _ in range(128):w(128)
  e.call(0x100389c84,d.dsp,36,86,doubles=(0.,))
  a=np.concatenate([w(128) for _ in range(48)],axis=1)
  e.call(0x100389c8c,d.dsp,36,0)
  b=np.concatenate([w(128) for _ in range(24)],axis=1)
  arms.append(np.concatenate((a,b),axis=1))
 assert not e.unknown and all(np.array_equal(x[0],x[1]) and np.max(x)>0 and np.min(x)<0 for x in arms)

 scope=Path("sj6-ipa-source-next")
 vol=json.loads((scope/'ipa-convergence.json').read_text())['ota_states']['0']['authored_brainwave'][1]
 active=Path('sj6-mixer-provenance/current.json')
 new=json.loads(active.read_text())['wrapper']
 assert np.float32(vol)==np.float32(new[str((0,.4141378700733185))])
 so=C.CDLL(str((scope/'source-mix.so').resolve()))
 class Bus(C.Structure):_fields_=[('l',P),('r',P)]
 fx=so.sj6_source_mix
 fx.argtypes=[C.c_uint,C.POINTER(Bus),P,P,F,P,P];fx.restype=C.c_int
 n=arms[0].shape[1]
 zero=np.zeros_like(arms[0])
 def ptr(x):return x.ctypes.data_as(P)
 def db(x,y):return 20*np.log10(max(np.linalg.norm(x.astype('f8')),1e-35)/np.linalg.norm(y.astype('f8')))
 res=[]
 for roles in ((0,),(1,),(0,1)):
  ins=[arms[i] if i in roles else zero for i in range(2)]+[zero,zero]
  out=np.empty((2,n),'f4')
  v=C.c_float*4;volumes=v(1,1,1,1);pans=v(0,0,0,0)
  busses=(Bus*4)(*[Bus(ptr(x[0]),ptr(x[1])) for x in ins])
  assert fx(n,busses,volumes,pans,vol,ptr(out[0]),ptr(out[1]))==1
  assert not fx(n,busses,volumes,v(0,.5,0,0),vol,ptr(out[0]),ptr(out[1]))
  # Source-connected unit stereo buses, final matched authored IPA gain.
  ref=(ins[0]+ins[1]+ins[2]+ins[3])*np.float32(vol)
  assert out.tobytes()==ref.tobytes()
  old=np.empty_like(out)
  for i,vals in enumerate(zip(ins[0][0],ins[1][0])):
   l,r=F(),F();fn((F*2)(*vals),C.byref(l),C.byref(r),1.,0.)
   old[:,i]=l.value,r.value
  res.append(dict(roles=roles,compared_words=2*n,compact_level_error_dB=db(old,ref),
                 peak=float(np.max(np.abs(out))),source_model_signed_sha=hashlib.sha256(ref.tobytes()).hexdigest()))
 print(json.dumps(dict(scope='Actual signed current Main receiver buffers, shared behavior from authenticated IPA and current resonance setter, Apple input volume 1 model; not real installed Apple PCM or integrated compact oscillator.',volume=vol,result=res),indent=2))
```

### ipa-room.py

```python
import ctypes as C,numpy as np,hashlib,sys,json
from pathlib import Path
import emu,macho
from unicorn.arm64_const import UC_CPU_ARM64_MAX
macho.EXPECTED_SHA='a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1'
old=emu.Uc
def mk(*a,**k):u=old(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=mk
D=Path(sys.argv[1]);n=32768
A_GLOBAL={'_sp_revsc_create':0x1003b3398,'_sp_revsc_init':0x1003b33c4,'_sp_revsc_compute':0x1003b3640,'_sp_revsc_destroy':0x1003b360c}
def original(x,cut,fb):
 e=emu.Emu(str(D/'mac-arm64'))
 A={k:e.im.sym_by_name[k] for k in A_GLOBAL}
 sp,ind,f=e.calloc(256),e.calloc(8),e.calloc(16)
 e.w32(sp+8,44100);e.w32(sp+12,2)
 assert e.call(A['_sp_revsc_create'],ind)==1
 s=e.r64(ind);assert e.call(A['_sp_revsc_init'],sp,s)==1
 e.wf(s,float(fb));e.wf(s+4,float(cut));assert e.r64(s+0x1a8)==98760
 y=np.empty_like(x)
 for i in range(n):
  e.wf(f,float(x[0,i]));e.wf(f+4,float(x[1,i]))
  assert e.call(A['_sp_revsc_compute'],sp,s,f,f+4,f+8,f+12)==1
  y[:,i]=e.rf(f+8),e.rf(f+12)
 damp=e.rf(s+0x18)
 e.call(A['_sp_revsc_destroy'],ind);assert not e.unknown,e.unknown
 return y,damp
ptr=np.ctypeslib.ndpointer(dtype=np.float32,flags='C_CONTIGUOUS')
def model(x,div,damp,fb):
 f=C.CDLL(str(D/f'model{div}.so')).run;f.argtypes=[ptr,ptr,C.c_uint,C.c_float,C.c_float];f.restype=C.c_uint
 y=np.empty_like(x);size=f(x,y,n,fb,damp);assert size and (size<=700 or div=='float28') and np.isfinite(y).all()
 return y,size
def dbr(x,y):
 a=float(np.linalg.norm(x));b=float(np.linalg.norm(y))
 return 20*np.log10(a/b) if a and b else None
def digest(x):return hashlib.sha256(x.astype('<f4').tobytes()).hexdigest()
def describe(y):
 t=np.flatnonzero(np.any(y!=0,axis=0))
 z=np.abs(np.fft.rfft(y,axis=1))**2;hz=np.fft.rfftfreq(n,1/44100)
 return dict(first_nonzero=int(t[0]) if len(t) else None,nonzero=int(len(t)),signed_sha=digest(y),
        above_8k=float(z[:,hz>8000].sum()/z.sum()) if z.sum() else None)
i=np.zeros((2,n),'f4');i[0,0]=.25;i[1,37]=-.125
t=np.arange(n);a=(t<7424)
d=np.stack([((t*3023%65536)-32768)/196608.*a,((t*1597%65536)-32768)/196608.*a]).astype('f4')
sec=t/44100.;en=np.minimum(1.,sec/.010)*np.minimum(1.,np.maximum(0.,(.257-sec)/.040))
h=np.stack([(.135*np.sin(261.625*2*np.pi*sec)+.072*np.sin(880.*2*np.pi*sec)+.021*np.sin(12853.*2*np.pi*sec))*en,
 (.12*np.sin(329.627*2*np.pi*sec+.2)+.063*np.sin(1302.2*2*np.pi*sec)+.027*np.sin(9565.*2*np.pi*sec))*en]).astype('f4')

for name,x,c,f in [('impulse',i,20000.,.7779527902603149),('dense',d,20000.,.7779527902603149),('held chord + release',h,20000.,.92),('held chord type-I',h,4000.,.92)]:
 y,v=original(x,c,f);print(json.dumps(dict(cue=name,ref=digest(y),cut=c,coeff=v,first=describe(y)['first_nonzero'])),flush=True)
```

### mix-result.json

```json
{
  "scope": "Actual signed current Main receiver buffers, shared behavior from authenticated IPA and current resonance setter, Apple input volume 1 model; not real installed Apple PCM or integrated compact oscillator.",
  "volume": 0.8670601844787598,
  "result": [
    {
      "roles": [
        0
      ],
      "compared_words": 18432,
      "compact_level_error_dB": -20.344609569511974,
      "peak": 0.31005531549453735,
      "source_model_signed_sha": "8cc08074ef8fe1ee47790e0a8a100388cae252c4428e8a04ed6e096f93f3cd8c"
    },
    {
      "roles": [
        1
      ],
      "compared_words": 18432,
      "compact_level_error_dB": -20.344609569515665,
      "peak": 0.3231624364852905,
      "source_model_signed_sha": "7addce9a4c7508fd6c7df506c3800559624c4e1ff8e730c1e30a514774c19313"
    },
    {
      "roles": [
        0,
        1
      ],
      "compared_words": 18432,
      "compact_level_error_dB": -20.344609577258307,
      "peak": 0.6325745582580566,
      "source_model_signed_sha": "4a8e66676dc3844168d0d40e969b1ad1746d67fda693e77c7245317fdb31aa25"
    }
  ]
}
```

### ipa-room.jsonl

```json
{"cue": "impulse", "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "cut": 20000.0, "coeff": 0.17418217658996582, "first": 1981}
{"cue": "dense", "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "cut": 20000.0, "coeff": 0.17418217658996582, "first": 1944}
{"cue": "held chord + release", "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "cut": 20000.0, "coeff": 0.17418217658996582, "first": 1945}
{"cue": "held chord type-I", "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "cut": 4000.0, "coeff": 0.5740326046943665, "first": 1945}
```

### ipa-normal-coeff.json

```json
{
  "scope": "Executed isolated old AU; first three frequencies are rounded capture-related prep targets, not asserted exact live callback values",
  "sr": 44100,
  "damping": 0.4043673574924469,
  "order": [
    "b0",
    "b1",
    "b2",
    "a1",
    "a2"
  ],
  "rows": [
    [
      699.264,
      [
        "0x1.3f6c2ef090a00p-9",
        "0x1.3f6c2ef090a00p-8",
        "0x1.3f6c2ef090a00p-9",
        "-0x1.f4a092ca2cf2cp+0",
        "0x1.ee3ed6501c281p-1"
      ]
    ],
    [
      229.779,
      [
        "0x1.17d8055433800p-12",
        "0x1.17d8055433800p-11",
        "0x1.17d8055433800p-12",
        "-0x1.fcc55c4eac5eep+0",
        "0x1.fa16a4a002d78p-1"
      ]
    ],
    [
      211.0,
      [
        "0x1.d80ba5863c800p-13",
        "0x1.d80ba5863c800p-12",
        "0x1.d80ba5863c800p-13",
        "-0x1.fd0dcf10ce967p+0",
        "0x1.fa91a10afebbfp-1"
      ]
    ],
    [
      -1.0,
      [
        "0x1.5fe4ad8680000p-20",
        "0x1.5fe4ad8680000p-19",
        "0x1.5fe4ad8680000p-20",
        "-0x1.ffd7ea3ee83b1p+0",
        "0x1.ffb0847027396p-1"
      ]
    ],
    [
      30000.0,
      [
        "0x1.fd097be24ab16p-1",
        "0x1.fd097be24ab16p+0",
        "0x1.fd097be24ab16p-1",
        "0x1.fce944d662023p+0",
        "0x1.fa5365dc66c12p-1"
      ]
    ]
  ],
  "history_bytes_unchanged": true
}
```

### ipa-normal-receiver.json

```json
{
  "scope": "Execute old authorized Swift senders -> property -> real DSP -> VoicePrep.\n\nMinimal fake platform object custody only: no Mac audio, app timer, media/UI,\nfull original graph, or LFO scheduling. Normal mode, no live cutoff writes.\n",
  "rate": 44100,
  "MIDI_receiver": [
    36,
    86
  ],
  "receiver": {
    "129": [
      "0x1000ed7cc",
      0.4000000059604645,
      36,
      300.0
    ],
    "130": [
      "0x1000ee1cc",
      0.4141378700733185,
      8,
      7.86447811126709
    ],
    "131": [
      "0x1000eddf4",
      0.5313359498977661,
      39,
      0.13283398747444153
    ],
    "132": [
      "0x1000ef1e8",
      0.5168501734733582,
      16,
      1.7785197496414185
    ],
    "133": [
      "0x1000ef3a0",
      0.47146177291870117,
      17,
      1.9349507093429565
    ],
    "134": [
      "0x1000ee16c",
      0.08876017481088638,
      18,
      0.0023471745662391186
    ],
    "135": [
      "0x1000ef558",
      0.604790449142456,
      19,
      4.653963088989258
    ],
    "136": [
      "0x1000edf88",
      0.4559931755065918,
      7,
      0.09260699152946472
    ],
    "137": [
      "0x1000eee54",
      0.9064822196960449,
      40,
      0.9971304535865784
    ]
  },
  "prep": [
    {
      "frames": 16,
      "voice_velocity": 0.6771653294563293,
      "base_pitch": 65.40638732910156,
      "separate_track": 0.0,
      "track": 0.13283398747444153,
      "strength": 0.09260699152946472,
      "velocity_scaling": 0.9971304535865784,
      "damping": 0.4043673574924469,
      "user_cutoff": 300.0,
      "filter_stage": 2
    },
    {
      "frames": 16,
      "voice_velocity": 0.6771653294563293,
      "base_pitch": 65.40638732910156,
      "separate_track": 0.0,
      "track": 0.13283398747444153,
      "strength": 0.09260699152946472,
      "velocity_scaling": 0.9971304535865784,
      "damping": 0.4043673574924469,
      "user_cutoff": 300.0,
      "filter_stage": 2
    },
    {
      "frames": 16,
      "voice_velocity": 0.6771653294563293,
      "base_pitch": 65.40638732910156,
      "separate_track": 0.0,
      "track": 0.13283398747444153,
      "strength": 0.09260699152946472,
      "velocity_scaling": 0.9971304535865784,
      "damping": 0.4043673574924469,
      "user_cutoff": 300.0,
      "filter_stage": 2
    }
  ],
  "cutoff_damping_targets": [
    [
      300.5797293186188,
      0.4043673574924469
    ],
    [
      301.1592575311661,
      0.4043673574924469
    ],
    [
      301.7385845184326,
      0.4043673574924469
    ]
  ],
  "filter_descriptor": [
    [
      0.0,
      0.0,
      0.0,
      -1
    ],
    [
      1.0,
      0.0,
      0.0,
      27
    ],
    [
      0.0,
      1.0,
      0.22313016014842982,
      4902
    ],
    [
      1.0,
      0.0023471745662391186,
      0.0070834089290521185,
      5333
    ],
    [
      0.0023471745662391186,
      0.0023471745662391186,
      0.0,
      -1
    ],
    [
      0.0023471745662391186,
      0.0,
      0.0070834089290521185,
      12827
    ]
  ],
  "filter_env_f32": [
    0.00042445710278116167,
    0.0008487669401802123,
    0.0012729294830933213
  ],
  "exact_cutoff_recurrence": true
}
```

### current.py

```python
"""Actual current already-entered resonance sender tail -> native Swift -> Apple API."""
import tarfile
from unicorn.arm64_const import UC_ARM64_REG_LR
from pathlib import Path
p=Path('sj6-mixer-next/constructor.py')
exec(compile(p.read_text().split('print(json.dumps')[0],str(p),'exec'))
stem=Path('sj6-mixer-next/private/SJ6-Brainwave-Direct-AUv3-Fanin.tgz')
assert hashlib.sha256(stem.read_bytes()).hexdigest()=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
with tarfile.open(stem) as t:
 m=json.load(t.extractfile('native-warm-brainwave-fanin-authored-all/capture.json'))
auth={int(x['address']):x['value'] for x in m['parameters']}
assert m['parameters']==m['parameter_readback_after']
obj.native_imps.update({method:dict(o.classes[c][1])[method] for method in ('setVolume:','setPan:')})
cond=e.calloc(0x1000);ins=e.calloc(0x1000)
e.w64(cond+0x40,ins); e.w64(ins+32,core)
e.w64(0x10082f448,0x50); e.w64(0x1007e3be0,0x700)
e.w64(core+0x700,obj.addr)
def entry(*_):
 e.setd(8,user); e.setd(9,1.)
def finish(*_):e.uc.emu_stop()
for at,fn in ((0x1000ef97c,entry),(0x1000ef9e8,finish)):
 e.uc.hook_add(UC_HOOK_CODE,fn,begin=at,end=at)
rows={}
for mode in (0,1):
 e.wr(cond+0x50,bytes([mode]))
 for user in (0.,auth[130],1.):
  values.clear()
  e.call(0x1000ef97c,regs={20:cond,24:0x40})
  assert len(values)==1 and values[0][0]=='setOutputVolume:',(values,e.unknown)
  rows[str((mode,user))]=values[0][1]
assert not e.unknown
print(json.dumps(dict(source=macho.EXPECTED_SHA,controls={j:auth.get(j) for j in (8,130,209)},wrapper=rows,scope='Actual current source sender tail at .0ef97c; initialized caller registers/owner by documented surrogate. No full preset dispatcher or audio graph claim. Actual external outputVolume argument observed; no pan message.'),indent=2))
```

### current.json

```json
{
  "source": "6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850",
  "controls": {
    "8": 0.4141378700733185,
    "130": 0.4141378700733185,
    "209": 0
  },
  "wrapper": {
    "(0, 0.0)": 1.0,
    "(0, 0.4141378700733185)": 0.8670601844787598,
    "(0, 1.0)": 0.5011872053146362,
    "(1, 0.0)": 1.0,
    "(1, 0.4141378700733185)": 0.8933950662612915,
    "(1, 1.0)": 0.6000000238418579
  },
  "scope": "Actual current source sender tail at .0ef97c; initialized caller registers/owner by documented surrogate. No full preset dispatcher or audio graph claim. Actual external outputVolume argument observed; no pan message."
}
```
