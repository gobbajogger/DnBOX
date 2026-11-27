# IPA pre-filter source order and stricter Room holdout

The original traced Main receiver applies the voice envelope at the already oversampled oscillator output before the Normal IIR. The compact prototype computes the Normal filter on the un-enveloped source and multiplies its output. This is a genuine order difference independent of the prototype /6 headroom decision. A host diagnostic moves the voice weight before the existing compact IIR only for Normal typed SuperJ6 Main-only. Legacy voice, active Sub/Noise and active OTA remain bit-identical under the compiled probe. Active Main-only branch changes 265,072 signed words with a small whole-render old/new residual (−66.76 dBr in this simple compact-filter probe). It cannot account for the main ~12 dB dry gap by itself, and this patch does not make the compact IIR equal the original or establish terminal parity. No source gain or preset level changed. Default-off, host-only.

Under the authentic IPA Normal sender chain, erroneously treating each internal short render as a fresh outer callback changes the physical envelope evolution. Signed measured first-release residual −8.856 / −6.844 dBr at keys 36/48 when a 128-frame outer cue is split to false 17-frame callback boundaries. That is an actual original negative control, not evidence that an exact DnBOX product callback did this. Keep outer callback time across internal subdivisions; the earlier guarded host does and passes its split control.

An independent stereo chirped/chord holdout also strengthens the Room choice: uniform 17 and the smaller mixed 16/17 layout fail supplemental high-band checks even though the earlier four cues pass. Uniform **18-bit 55,568 B** is the lowest tested width that clears the expanded isolated matrix; 20-bit 61,736 B gives more than 11 dB extra HF margin. This supersedes the earlier minimum-of-four-cues figure, not proof of a universal bound or app fit. Factoring common vs private lines did not save memory at accepted quality; stronger fixed quantizer shaping also did not pass. No deployment.


### candidate.patch

```diff
--- a/firmware/src/dsp/dnbx_core.c
+++ b/firmware/src/dsp/dnbx_core.c
@@ -1028,9 +1028,21 @@
                 else
 #endif
                 {
+#if defined(X0X_HOST) && DNBOX_LAB_SJ6_AMP_BEFORE_NORMAL
+                const int source_main=s->superj6_amp_range && p->oscillator_mode==DNBX_OSC_SUPERJ6
+#if X0X_SJ6_NOISE_SUB_NATIVE
+                  && s->sj6_noise_gain==0.f && s->sj6_sub_gain==0.f
+#endif
+                  && p->sub_gain==0.f;
+                if(source_main)x*=velocity*amp;
+#endif
                 lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);
                 lv->filter_second=fm_clampf(lv->filter_second+a*(lv->filter_first-lv->filter_second),-4,4);
+#if defined(X0X_HOST) && DNBOX_LAB_SJ6_AMP_BEFORE_NORMAL
+                mix[l]+=source_main?lv->filter_second:velocity*amp*lv->filter_second;
+#else
                 mix[l]+=velocity*amp*lv->filter_second;
+#endif
                 }
                 lv->phase+=lv->increment; if (lv->phase>=1) lv->phase-=1;
                 lv->subphase+=lv->increment*.5f; if (lv->subphase>=1) lv->subphase-=1;
```

### check.c

```c
#include "dnbx_core.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static dnbx_part s;
int main(int argc,char**argv){
    const unsigned mode=argc>1?(unsigned)atoi(argv[1]):0u;
    float a[129],b[129];
    unsigned total=0,active=0;
    for(unsigned c=0;c<8;c++){
     assert((mode==1?dnbx_init(&s,44100.f):dnbx_init_superj6(&s,44100.f))==DNBX_OK);
     for(unsigned k=0;k<2;k++){
      dnbx_layer_params p=s.layer[k].params;
      p.oscillator_mode=mode==1?DNBX_OSC_STOCK_BLEP:DNBX_OSC_SUPERJ6;
      p.waveform=k?1.f/3.f:2.f/3.f;p.sub_gain=mode==2?.12f:0.f;
      p.amp.release_s=.45f;
      assert(dnbx_set_layer(&s,k,&p)==DNBX_OK);
     }
     if(mode==3){const uint8_t aux[2]={10,24};dnbx_set_sj6_sources(&s,0,aux);}
     if(mode==4)s.sj6_ota=1;
     for(unsigned k=0;k<(c&1?4u:1u);k++)assert(dnbx_note_on(&s,36+5*k,86+k)>=0);
     for(unsigned j=0;j<256;j++){
      const unsigned n=1+(j*19)%129;
      if(j==80)for(unsigned k=0;k<(c&1?4u:1u);k++)assert(dnbx_note_off(&s,36+5*k)==DNBX_OK);
      if(j==190)assert(dnbx_note_on(&s,59,109)>=0);
      assert(dnbx_render_pitch_osc_levels_velocity(&s,a,b,n,1.f,.808f,.808f,0.f)==DNBX_OK);
      fwrite(a,sizeof(float),n,stdout);fwrite(b,sizeof(float),n,stdout);
      total+=n;for(unsigned k=0;k<n;k++)active+=(a[k]!=0||b[k]!=0);
     }
    }
    return active<total/2;
}
```

### result.json

```json
{
  "typed_normal_signed_words": 265072,
  "old_new_dbr": -66.76469209799737,
  "peak_candidate": 0.0837169885635376,
  "guarded_modes": [
    "legacy",
    "voice sub active",
    "SJ6 sub/noise active",
    "OTA"
  ]
}
```

### clock.py

```python
"""Run IPA initialized authored Normal projection; compare only callback clock."""
from pathlib import Path
import hashlib,numpy as np
p=Path('sj6-normal-source-recovery/normal_filter_receiver_oracle.py')
exec(compile(p.read_text().split('prep=[]')[0],str(p),'exec'))
pos=0; tick=[]
def tap(*_):
 if E.r32(E.x(0)+0x340): tick.append([pos,E.r32(E.x(0)+0x3a8),E.rd_(E.x(0)+0x370)])
E.uc.hook_add(UC_HOOK_CODE,tap,begin=0x1003a5664,end=0x1003a5664)
out=[]
for key in (36,48):
 rings=[]
 for split in (False,True):
  d=SamplerDSP(E,44100);d.load_placeholder()
  # Exact, already reached AU receiver types/words from this program's senders.
  for addr,value in D.param_log:d.set_param(addr,value,True)
  E.w32(d.dsp+0x58,4)
  d.set_param(15,.45,True)
  for _ in range(128):d.render(128)
  tick.clear();pos=0
  E.call(0x10037d148,d.dsp,key,86)
  a=[]
  for stage,n in enumerate((22050,8192)):
   if stage:E.call(0x10037d150,d.dsp,key,0)
   for j in range(0,n,128):
    size=min(128,n-j)
    # Internal split accidentally advanced as a new external render.
    for k in range(0,size,17 if split else 128):
     step=min(size-k,17 if split else 128)
     a.append(d.render(step));pos+=step
  rings.append([np.concatenate(a,axis=1),tick.copy()])
 x,a=rings[0];y,b=rings[1]
 dbr=lambda a,b:float(20*np.log10(np.linalg.norm((a-b).astype('f8'))/np.linalg.norm(a.astype('f8'))))
 out.append(dict(key=key,boundaries=[len(a),len(b)],first_ticks=[a[:4],b[:4]],
  next_off=[next(t for t in a if t[0]==22050),next(t for t in b if t[0]==22050)],
  compact_scope='Clock metamorphic negative; not same as shipping internal split or terminal PCM.',
  signed_residual={name:dbr(x[:,l:r],y[:,l:r]) for name,l,r in [('attack',0,512),('held',4096,22050),('first_release',22050,22562),('tail',22562,30242)]}))
assert not E.unknown
print(json.dumps({'scope':__doc__,'oracle_sha':sha256(Path(paths.BIN).read_bytes()).hexdigest(),'cases':out},indent=2))
```

### clock.json

```json
{
  "scope": "Execute old authorized Swift senders -> property -> real DSP -> VoicePrep.\n\nMinimal fake platform object custody only: no Mac audio, app timer, media/UI,\nfull original graph, or LFO scheduling. Normal mode, no live cutoff writes.\n",
  "oracle_sha": "a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1",
  "cases": [
    {
      "key": 36,
      "boundaries": [
        1891,
        3544
      ],
      "first_ticks": [
        [
          [
            0,
            2,
            0.0
          ],
          [
            0,
            2,
            0.00042445711664674054
          ],
          [
            0,
            2,
            0.0008487669359331984
          ],
          [
            0,
            2,
            0.0012729295089752848
          ]
        ],
        [
          [
            0,
            2,
            0.0
          ],
          [
            0,
            2,
            0.00042445711664674054
          ],
          [
            17,
            2,
            0.0008487669359331984
          ],
          [
            17,
            2,
            0.0012729295089752848
          ]
        ]
      ],
      "next_off": [
        [
          22050,
          5,
          0.4652406513175238
        ],
        [
          22050,
          5,
          0.7242831069190802
        ]
      ],
      "compact_scope": "Clock metamorphic negative; not same as shipping internal split or terminal PCM.",
      "signed_residual": {
        "attack": -34.23262856365124,
        "held": -7.63667889495883,
        "first_release": -8.85582910706469,
        "tail": -5.861853960583807
      }
    },
    {
      "key": 48,
      "boundaries": [
        1891,
        3544
      ],
      "first_ticks": [
        [
          [
            0,
            2,
            0.0
          ],
          [
            0,
            2,
            0.00042445711664674054
          ],
          [
            0,
            2,
            0.0008487669359331984
          ],
          [
            0,
            2,
            0.0012729295089752848
          ]
        ],
        [
          [
            0,
            2,
            0.0
          ],
          [
            0,
            2,
            0.00042445711664674054
          ],
          [
            17,
            2,
            0.0008487669359331984
          ],
          [
            17,
            2,
            0.0012729295089752848
          ]
        ]
      ],
      "next_off": [
        [
          22050,
          5,
          0.4652406513175238
        ],
        [
          22050,
          5,
          0.7242831069190802
        ]
      ],
      "compact_scope": "Clock metamorphic negative; not same as shipping internal split or terminal PCM.",
      "signed_residual": {
        "attack": -21.97175325881245,
        "held": -5.397784794420475,
        "first_release": -6.843547134680678,
        "tail": -4.989481753840729
      }
    }
  ]
}
```

### README.md

```text
# Physically mixed RevSC history precision: held-out rejection
Compiled portable full-rate eight original lines, independently sized 16/17-bit coded histories on their actual aligned allocations, no change to clock, taps, damping or sample math. Same immutable mask for all presets/inputs. Mixed layout all-16/all-17 proves exact output hashes to previous C arm on all four original cues. Original authenticated Build9 raw comparator under Unicorn. Bit 1 chooses 17 bits for that physical line; default experiment only, no product.

19 masks evaluated, including one downgrade per physical line from 0xff, both parity groups, and all child masks of qualified smaller combinations. Training accepts mask 0xfd only among reduced layouts: line index 1 gets 16 bits, others 17, 52,128 B history vs 52,484 B uniform, saving 356 B before the 8 B lab metadata. On the four original signed cues the worst HF residue was only 0.059 dB inside the provisional -60 dBr threshold. All combinations with two reduced lanes failed.

A separate held tones, swept-treble and stereo transient cue (feedback .885) was evaluated after freezing mask selection, at both actual 4k and 20k damping regimes. It did not affect mask selection. It rejected the mixed mask (HF residual -53.86/-53.66 dBr), and also rejected the old uniform 17-bit candidate (-54.89/-54.70). Both raw and late were comfortably below the threshold. Uniform 18-bit history 55,568 B passes both independent arms (-60.82/-60.62 dBr HF, raw -89.90/-85.25). Uniform 20-bit 61,736 B leaves >11 dB of HF margin; Float32 reference comparison floor -81.36/-77.23. Absolute late output levels already match closely. This is an isolated portable host result and not an architectural universal lower bound, firmware fit, or production guarantee.

Reproduce in the same private provisioned Build9 environment with replay.sh. Archive contains only source and signed hash/metric receipts.
```

### wider.py

```python
from pathlib import Path
from unicorn import UC_HOOK_CODE
base=Path(__file__).resolve().with_name("fixture-parent.py")
exec(compile(base.read_text().split('rows=[]')[0],str(base),'exec'))
def errors(y,ref):
 a=y.astype('f8');b=ref.astype('f8');err=a-b
 bands=np.fft.rfftfreq(n,1/44100)>8000
 return dict(raw=dbr(err,b),late=dbr(err[:,16000:],b[:,16000:]),early=dbr(err[:,:4096],b[:,:4096]),
             late_level=dbr(a[:,16000:],b[:,16000:]),hf=dbr(np.fft.rfft(err,axis=1)[:,bands],np.fft.rfft(b,axis=1)[:,bands]),
             first=describe(a)['first_nonzero'])
cues=[('impulse',i,20000,.7779527902603149),('dense',d,20000,.7779527902603149),('held chord + release',h,20000,.92),('held chord type-I',h,4000,.92)]
t=np.arange(n)/44100.;ev=np.minimum(t/.003,1)*np.minimum(np.maximum(.328-t,0)/.019,1)
p=np.minimum(np.maximum(t-.135,0)/.004,1)*np.minimum(np.maximum(.351-t,0)/.028,1)
sine=lambda f:np.sin(2*np.pi*f*t)
chirp=lambda p:np.sin(2*np.pi*(1400*t+p*t*t))
hs=np.stack([ev*(.093*sine(169)+.048*sine(691)+.012*chirp(15100))+.045*p*sine(4509), ev*(.062*sine(261)+.013*chirp(17600))+.11*p*sine(390)]).astype('f4')
for cut in (4000,20000):
 ref,_=original(hs,cut,.885)
 for m in (12,14,0):
  lib=C.CDLL(str(D/(f"p{m}.so" if m else "portable-revsc.so")))
  f=lib.runportable;f.argtypes=[ptr,ptr,C.c_uint,C.c_float,C.c_float];f.restype=C.c_uint
  out=np.empty_like(hs);size=f(hs,out,n,.885,cut)
  r=errors(out,ref)
  passes=r['first']==describe(ref)['first_nonzero'] and max(r[k] for k in ('raw','early','late','hf'))<=-60 and abs(r['late_level'])<=.1
  print(json.dumps(dict(cue="heldout",m=m,cut=cut,ref=digest(ref),signed_input=digest(hs),portable=digest(out),size=size,pass_all=bool(passes),**r)),flush=True)
```

### wider.jsonl

```json
{"cue": "heldout", "m": 12, "cut": 4000, "ref": "26bb1de9638a3f99d22f8494426d28282af4a00b2c4894f5ff723820bdddd2cb", "signed_input": "2bf9f3cc84260ae6555c0e13eb3bc8d23ccc71361ad5e1f8f1a6b9678c70ed25", "portable": "e2b9cccfaf537ba2ff752599899dcd022e71651a65695c4fbfbfb05cb4b0e398", "size": 55568, "pass_all": true, "raw": -89.90349522802389, "late": -88.82422999369452, "early": -94.29400015463189, "late_level": -4.365241927718983e-07, "hf": -60.82030024355909, "first": 1945}
{"cue": "heldout", "m": 14, "cut": 4000, "ref": "26bb1de9638a3f99d22f8494426d28282af4a00b2c4894f5ff723820bdddd2cb", "signed_input": "2bf9f3cc84260ae6555c0e13eb3bc8d23ccc71361ad5e1f8f1a6b9678c70ed25", "portable": "c0140e1cf10c7d7e33b1685784420070fbb23f0f3f8f7e9267c6293769f30efb", "size": 61736, "pass_all": true, "raw": -98.81352687433971, "late": -98.14534166902122, "early": -104.94795224911003, "late_level": -5.129526300656427e-06, "hf": -72.3619124931376, "first": 1945}
{"cue": "heldout", "m": 0, "cut": 4000, "ref": "26bb1de9638a3f99d22f8494426d28282af4a00b2c4894f5ff723820bdddd2cb", "signed_input": "2bf9f3cc84260ae6555c0e13eb3bc8d23ccc71361ad5e1f8f1a6b9678c70ed25", "portable": "7ea252092f278f4a8d101f25e59131cb195e81f591ffccb2167b68c6ceb7c967", "size": 98760, "pass_all": true, "raw": -101.65109377390391, "late": -101.51533846770278, "early": -110.33186371735825, "late_level": -6.134774997410052e-06, "hf": -81.36297639241502, "first": 1945}
{"cue": "heldout", "m": 12, "cut": 20000, "ref": "169a519ca7da67d1adeb5177001c7ecd14883a9d813a8ab0750db00ee46d9378", "signed_input": "2bf9f3cc84260ae6555c0e13eb3bc8d23ccc71361ad5e1f8f1a6b9678c70ed25", "portable": "2364fa00d6ebb52c142000161a55e425e2770d1028f14d3f130a812b7ec13ee8", "size": 55568, "pass_all": true, "raw": -85.24711465632977, "late": -83.94249946371471, "early": -90.789385817462, "late_level": 1.6133378887705614e-07, "hf": -60.618503955047785, "first": 1945}
{"cue": "heldout", "m": 14, "cut": 20000, "ref": "169a519ca7da67d1adeb5177001c7ecd14883a9d813a8ab0750db00ee46d9378", "signed_input": "2bf9f3cc84260ae6555c0e13eb3bc8d23ccc71361ad5e1f8f1a6b9678c70ed25", "portable": "487577ec2eb1b34977e2b091288317ffac0718781aca13ae7970cf4d00893b9b", "size": 61736, "pass_all": true, "raw": -93.40205893227888, "late": -92.03167560715411, "early": -102.08859258398121, "late_level": 7.082970545988597e-07, "hf": -71.47290922853796, "first": 1945}
{"cue": "heldout", "m": 0, "cut": 20000, "ref": "169a519ca7da67d1adeb5177001c7ecd14883a9d813a8ab0750db00ee46d9378", "signed_input": "2bf9f3cc84260ae6555c0e13eb3bc8d23ccc71361ad5e1f8f1a6b9678c70ed25", "portable": "4df6afc5777817fafd20304118ca527491341d9d2f5e1be29acafb2189cd9181", "size": 98760, "pass_all": true, "raw": -95.30494634451762, "late": -93.90946674394911, "early": -109.9547049889747, "late_level": 6.552304789381512e-07, "hf": -77.2288110043987, "first": 1945}
```
