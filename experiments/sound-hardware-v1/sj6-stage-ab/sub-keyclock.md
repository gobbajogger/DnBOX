# SuperJ6 Sub: trace the small held-note pitch drift to authored key ranges

Current pinned original AU and exact authorized original Sub map, local-only. The independent dry Main/Sub output ratio pointed to a small phase drift not explained by the shared magnitude gap. Two fixed absolute Hann centers on Brainwave key 36 measure compact projected minus original Sub H1 of **+0.017309/+0.017326 Hz**, with similar H3/3 values. Main is approximately neutral (−0.00035 to −0.00180 Hz).

The original graph's Sub map binding and note dispatch identify a per-key tuning lookup with a signed cents-to-semitones conversion. Rerunning the current source sampler with the same held cue changes the actual normalized cursor step **0.999999881 → 0.999442637** under the canonical key>range offset. Nominal minus canonical +0.018228/+0.018201 Hz, within 0.001 Hz of the original measured H1 divergence, and correct harmonic scaling. Held-out key 55 demonstrates authentic shift in the opposite direction (~−0.02030 Hz); loaded chord captures have a larger mixed/handoff residual, not explained by this one factor. At key 32 zero/missing offset is bit-exact. No master volume or preset edits. This does not explain the approximately 12 dB dry magnitude gap.

The independent proposed optional immutable milli-cent profile inlet (code only, no built-in data) is sealed from the earlier exact warm core. The local native/source branch may be older; do not apply blindly. Keep feature off until source preimages, immutable profile provenance, caller owner binding, native fit and end-to-end signed qualification are proven. There is no shipping change and this does not approximate a generated Sub to an exact sample.

## Evidence and replay

Use the authorized SHA-pinned original assets at the paths identified by the local bench. Do not store original executable, map payload or captures with this receipt. Run the model check for keys 32, 36 and 55 independently, then compare `phase-drift.py` against the existing source/terminal signed references.

### sub-clock.py

```python
"""Private exact sampled Sub receiver, no protected sample or clock-table export."""
from pathlib import Path
import zipfile,hashlib,numpy as np,struct,zlib,re,json
a=Path('sj6-mix-boundary-recheck/wrapper.py');exec(compile(a.read_text().split('def sampler_owner')[0],str(a),'exec'))
archive=next(Path('ab-private/superj6').glob('*.ipa'))
with zipfile.ZipFile(archive) as z:
 asset=lambda suffix:z.read(next(n for n in z.namelist() if n.endswith(suffix)))
 sfz=asset('/Sounds/Osc/MySub.sfz');pack=asset('/J6P8R4N2S1')
sha=lambda b:hashlib.sha256(b).hexdigest()
assert sha(pack)=='ec994e3fabf42b2761d7ca3799f87dacb4ac12670e2909451ff14f78b9a5f740'
assert sha(sfz)=='3668ef83c102b4952a19bc6fd5adc585eb69d0eff4dd4743be9488b0771fc75d'
import sys
group={};reg=None;key=int(sys.argv[1]) if len(sys.argv)>1 else 36
for row in sfz.decode().splitlines():
 kv=dict(re.findall(r'(\w+)=(\S+)',row))
 if row.strip().startswith('<group>'):group=kv
 if row.strip().startswith('<region>'):
  r=group|kv
  if int(r['lokey'])<=key<=int(r['hikey']):reg=r
assert reg
p0=16; region_name=reg['sample'].replace(chr(92),'/').split('/')[-1]
for i in range(struct.unpack_from('<I',pack,12)[0]):
 n,=struct.unpack_from('<H',pack,p0);p0+=2
 name=pack[p0:p0+n].decode();p0+=n
 frames,rate,off,nb,crc=struct.unpack_from('<IIQQI',pack,p0);p0+=28
 if name.split('/')[-1]==region_name:
  payload=pack[off:off+nb];assert zlib.crc32(payload)==crc and nb==2*frames
  sample=np.frombuffer(payload,'<i2').astype('f4')/32768.;break
else:raise ValueError('unmatched exact source')
binary=BIN.read_bytes();start=0x7e3c78
pairs=dict(struct.unpack_from('<Qd',binary,start+32+16*i) for i in range(struct.unpack_from('<Q',binary,start+16)[0]))
# This is the constructor's authenticated row and signed .2b9aa8 scale.
# Do not copy the protected table into a firmware implementation.
st=-(pairs.get(key,0.)/100)
del pairs
def act(detune):
 d=sampler.SamplerDSP(e,44100)
 root=int(reg['pitch_keycenter'])
 d.load_sample(dict(note=root,freq=440*2**((root-69)/12),lokey=int(reg['lokey']),hikey=int(reg['hikey']),loop=True,loop_start=float(reg['loop_start']),loop_end=float(reg['loop_end']),rate=rate),sample)
 d.build_keymap()
 for addr,val in p['sampler1']:d.set_param(addr,val,True)
 for q in (2,3):d.set_param(q,0.,True)
 for n in (128,)*128:d.render(n)
 e.call(0x100389c84,d.dsp,key,86,doubles=(detune,))
 track=[]
 x=[]
 def tap(*_):
  if len(track)<2: track.append(struct.unpack('<d',e.rd(e.x(24)+0x18,8))[0])
 h=e.uc.hook_add(UC_HOOK_CODE,tap,begin=0x1003b78e0,end=0x1003b78e0)
 for n in (128,)*173:x.append(d.render(n))
 e.uc.hook_del(h)
 return np.concatenate(x,axis=1),track
zero,zs=act(0.); tune,ts=act(st)
f=440*2**((key-12-69)/12);W=((4096,10240),(9216,15360),(14336,20480))
def metric(a,m):
 c=np.array([a[0,l:r]@(np.hanning(r-l)*np.exp(-2j*np.pi*f*m*np.arange(l,r)/44100)) for l,r in W])
 return c
rows=[]
for m in (1,3,5):
 ratio=metric(zero,m)/metric(tune,m)
 rows.append(dict(h=m,measured_zero_minus_tuned_hz=(np.angle(ratio[1:]/ratio[:-1])*44100/(5120*2*np.pi*m)).tolist()))
assert not e.unknown and zs and ts and bool(np.any(tune!=zero))==bool(st)
print(json.dumps(dict(scope='Genuine current sampled receiver on exact authorized mapped key media, projected Brainwave RAM controls, fixed independent pitch, signed source modifier only. No app Apple graph, copy or production apply.',src=macho.EXPECTED_SHA,key=key,signed_sha=[sha(zero.tobytes()),sha(tune.tobytes())],nominal_minus_tuned_expected_hz=f*(1-2**(st/12)),core_cursor_step_z_t=[zs,ts],rows=rows),indent=2))
```

### phase-drift.py

```python
"""Fixed-bin harmonic phase growth, no fitted shift/ratio applied to audio."""
from pathlib import Path
p=Path('sj6-mixer-next/sub-ratio.py')
exec(compile(p.read_text().split('def ratio(z,w):')[0],str(p),'exec'))
hz0=440*2**((24-69)/12)
# Half-overlap is intentional: establish stable observed slope on several
# disjoint center spacings independently of first-vs-late window shape.
W=((20480,26624),(25600,31744),(30720,36864))
def phasors(z,M):
 return np.array([z[:,i:j]@(np.hanning(j-i)*np.exp(-2j*np.pi*hz0*M*np.arange(i,j)/44100.)) for i,j in W])
d={}
for m in (1,3,5,7):
 out={}
 for role,native,conditional in (('Main',x,u),('Sub',y,v)):
  A,B=phasors(native,m),phasors(conditional,m)
  rel=B/A
  delta=lambda c:np.angle(c[1:]/c[:-1])*44100/(5120*2*np.pi*m)
  out[role]=dict(same_input_hz_per_harmonic_native=delta(A).tolist(),same_input_hz_per_harmonic_projected=delta(B).tolist(),effective_projected_minus_native_hz=delta(rel).tolist(),phase_gap_deg=np.angle(rel,deg=True).tolist())
 d[m]=out
print(json.dumps(dict(scope='Brainwave independently captured signed current native and conditional original-leaf suffix. Fixed absolute frequencies/window spacing only, no PCM alignment or phase correction; effective frequency is not directly observed sampler step.',native_hashes=[a['pcm_sha256'],b['pcm_sha256']],cases=d),indent=2))
```

### chord-phase.py

```python
from pathlib import Path
p=Path('sj6-mixer-next/sub-ratio.py')
exec(compile(p.read_text().split('def ratio(z,w):')[0],str(p),'exec'))
import json
W=((55200,61344),(60320,66464),(65440,71584),(70560,76704))
f=440*2**((55-12-69)/12)
def cov(z,k):
 return np.array([z[0,i:j]@(np.hanning(j-i)*np.exp(-2j*np.pi*np.arange(i,j)*f*k/44100.)) for i,j in W])
out={}
for k in (1,3,5):
 native,recon=cov(y,k),cov(v,k)
 ratio=recon/native
 out[k]={'freq':f*k,'relative_hz_per_harmonic':(np.angle(ratio[1:]/ratio[:-1])*44100/(5120*2*np.pi*k)).tolist(),'relative_phase':np.angle(ratio,deg=True).tolist()}
print(json.dumps(out,indent=2))
```

### sub-clock-32.json

```json
{
  "scope": "Genuine current sampled receiver on exact authorized mapped key media, projected Brainwave RAM controls, fixed independent pitch, signed source modifier only. No app Apple graph, copy or production apply.",
  "src": "6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850",
  "key": 32,
  "signed_sha": [
    "ebcc98a0891fb7072463056bbf1791bc91e9b895de793bb0d77919f96ad4588d",
    "ebcc98a0891fb7072463056bbf1791bc91e9b895de793bb0d77919f96ad4588d"
  ],
  "nominal_minus_tuned_expected_hz": 0.0,
  "core_cursor_step_z_t": [
    [
      1.1224621534347534,
      1.1224621534347534
    ],
    [
      1.1224621534347534,
      1.1224621534347534
    ]
  ],
  "rows": [
    {
      "h": 1,
      "measured_zero_minus_tuned_hz": [
        0.0,
        0.0
      ]
    },
    {
      "h": 3,
      "measured_zero_minus_tuned_hz": [
        0.0,
        0.0
      ]
    },
    {
      "h": 5,
      "measured_zero_minus_tuned_hz": [
        -1.5404259016946733e-17,
        3.172985929623532e-17
      ]
    }
  ]
}
```

### sub-clock-36.json

```json
{
  "scope": "Genuine current sampled receiver on exact authorized mapped key media, projected Brainwave RAM controls, fixed independent pitch, signed source modifier only. No app Apple graph, copy or production apply.",
  "src": "6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850",
  "key": 36,
  "signed_sha": [
    "3f46f28aac67b9fd877ba5f1b9b6c0a3eb5b1199591aa263c646bc99755b04d6",
    "34bebfce8b3f5d7ecea7f418f027682236b473ffee4c38006703887b3fec8e5e"
  ],
  "nominal_minus_tuned_expected_hz": 0.01822387330912251,
  "core_cursor_step_z_t": [
    [
      0.9999998807907104,
      0.9999998807907104
    ],
    [
      0.9994426369667053,
      0.9994426369667053
    ]
  ],
  "rows": [
    {
      "h": 1,
      "measured_zero_minus_tuned_hz": [
        0.018228069347802616,
        0.01820051257388464
      ]
    },
    {
      "h": 3,
      "measured_zero_minus_tuned_hz": [
        0.01821463925443098,
        0.018260340228858037
      ]
    },
    {
      "h": 5,
      "measured_zero_minus_tuned_hz": [
        0.01827271541134832,
        0.018261777468939366
      ]
    }
  ]
}
```

### sub-clock-55.json

```json
{
  "scope": "Genuine current sampled receiver on exact authorized mapped key media, projected Brainwave RAM controls, fixed independent pitch, signed source modifier only. No app Apple graph, copy or production apply.",
  "src": "6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850",
  "key": 55,
  "signed_sha": [
    "e45136b27352bc9724255739caf39bfaa9da138d0a802d175385ff9b7a6cf14b",
    "f54453e33ed4bdbc24d218aca72286b7838a4688951250338acce6fd30652954"
  ],
  "nominal_minus_tuned_expected_hz": -0.02026717256292184,
  "core_cursor_step_z_t": [
    [
      1.0594631433486938,
      1.0594631433486938
    ],
    [
      1.0596823692321777,
      1.0596823692321777
    ]
  ],
  "rows": [
    {
      "h": 1,
      "measured_zero_minus_tuned_hz": [
        -0.020299707582041082,
        -0.020303027248906886
      ]
    },
    {
      "h": 3,
      "measured_zero_minus_tuned_hz": [
        -0.020515232133904406,
        -0.020378232496666805
      ]
    },
    {
      "h": 5,
      "measured_zero_minus_tuned_hz": [
        -0.019518219010708198,
        -0.020574150098913876
      ]
    }
  ]
}
```

### candidate.patch

```diff
--- a/firmware/src/dsp/dnbx_core.c
+++ b/firmware/src/dsp/dnbx_core.c
@@ -928,6 +928,28 @@
     {65,12,7,4},{56,14,8,5},{62,16,9,7},{49,14,7,4},
     {53,16,9,6},{52,12,3,1},{32,3,1,1},{35,2,0,0}
 };
+#if X0X_SJ6_SUB_TUNE_PROFILE
+static inline int32_t sj6_sub_tune_mc(const dnbx_sj6_sub_tune_profile *p,unsigned k)
+{
+    const uint8_t *b=p->milli_cents[k];
+    uint32_t v=(uint32_t)b[0]|((uint32_t)b[1]<<8)|((uint32_t)b[2]<<16);
+    return v<0x800000u?(int32_t)v:-(int32_t)(0x1000000u-v);
+}
+int dnbx_sj6_bind_sub_tune_profile(dnbx_part *s,const dnbx_sj6_sub_tune_profile *p,size_t n)
+{
+    if(!valid(s)||!s->superj6_amp_range||dnbx_active_voices(s))return 0;
+    if(!p) {
+        if(n)return 0;
+        s->sj6_sub_tune_profile=0;return 1;
+    }
+    if(n!=sizeof(*p)||p->version!=1||p->units!=1||p->reserved[0]||p->reserved[1])return 0;
+    for(unsigned k=0;k<128;k++){
+        int32_t v=sj6_sub_tune_mc(p,k);
+        if(v< -100000 || v>100000)return 0;
+    }
+    s->sj6_sub_tune_profile=p;return 1;
+}
+#endif
 static unsigned sj6_sub_zone(unsigned note)
 { return note<34u ? 0u : note>=94u ? 11u : (note-34u)/6u+1u; }
 static inline float sj6_synthesize_sub(float phase,float step,const uint8_t *h)
@@ -975,7 +997,12 @@
             unsigned note=s->voice[i].note;
             const unsigned ranged=(unsigned)((int)note+shift);
             if(ranged<=127u)note=ranged;
-            sj6_steps[i]=.5f*pitch_increment(s,note,0.f)*source_bend;
+            float source_offset=0.f;
+#if X0X_SJ6_SUB_TUNE_PROFILE
+            if(s->sj6_sub_tune_profile)
+                source_offset=(float)sj6_sub_tune_mc(s->sj6_sub_tune_profile,note)*.00001f;
+#endif
+            sj6_steps[i]=.5f*pitch_increment(s,note,source_offset)*source_bend;
             sj6_harmonics[i]=sj6_sub_harmonic[sj6_sub_zone(note)];
         }
     }
--- a/firmware/src/dsp/dnbx_core.h
+++ b/firmware/src/dsp/dnbx_core.h
@@ -142,6 +142,10 @@
 #endif
 #if X0X_SJ6_NOISE_SUB_NATIVE
     float sj6_noise_gain,sj6_sub_gain;
+#if X0X_SJ6_SUB_TUNE_PROFILE
+    const struct dnbx_sj6_sub_tune_profile *sj6_sub_tune_profile;
+#endif
+
     uint32_t sj6_noise_random;
     uint8_t sj6_noise_code,sj6_sub_code;
 #endif
@@ -207,6 +211,17 @@
 #endif
 #if X0X_SJ6_NOISE_SUB_NATIVE
 void dnbx_set_sj6_sources(dnbx_part *,unsigned owner,const uint8_t code[2]);
+#if X0X_SJ6_SUB_TUNE_PROFILE
+/* Caller owns already-authorized, immutable RAM or memory-mapped storage
+ * for this part's lifetime. This leaf validates format, not provenance.
+ * Packed signed 24-bit little-endian milli-cents indexed by physical key.
+ * NULL/0 detaches; only a silent typed SJ6 owner can rebind. */
+typedef struct dnbx_sj6_sub_tune_profile {
+    uint8_t version, units, reserved[2], milli_cents[128][3];
+} dnbx_sj6_sub_tune_profile;
+int dnbx_sj6_bind_sub_tune_profile(dnbx_part *,const dnbx_sj6_sub_tune_profile *,size_t);
+#endif
+
 #endif
 #if X0X_SJ6_VINTAGE_NATIVE
 float dnbx_sj6_vintage_cents(unsigned position);
```
