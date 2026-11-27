# Require High-pass and Unison source order on the guarded stereo inlet

The original host-only source inlet bypasses juno6_render_stereo, which otherwise owns the instrument High-pass. Its first effects-off test did not cover that dependency. Composing the actual pinned source-order route explicitly moves Unison then High-pass into sj6_run_effects, which the inlet calls. Without it, the negative-control inlet returned valid but wrong PCM when High-pass was active (peak signed delta 0.37645, or 0.24654 with Unison on).

The guarded inlet now refuses before writing audio if an active High-pass or Unison cannot be routed in source order. Scope is host-only; this is not a change to the stock mono fallback, the physical keyboard or release. Two stereo pilots (left-only and anti-phase), Unison on/off and active High-pass. Four accepted composed cases match the explicit signed 512-word reference stages bit for bit at the runtime boundary; four uncomposed cases refuse with exact untouched output sentinels. Left-only has exact zero right; anti-phase survives as nontrivial signed side, no mono collapse. Source-order trace 0x912 means summed input, then Unison, then HP. This is same-kernel wiring verification, not matching exact real IPA Unison/HP DSP or whole preset output.

Parent decision: reproduce the suggested dependence against the original guarded donor and actual C arrangement, add a fail-closed guard, preserve positive and negative controls, no new release or hardware claim. Research note path unavailable in this workspace; decision retained here with the implementation.

Reproduce from the host mixer inlet and the pinned source-order app/dsp donor. Apply ordered.patch in the app overlay, then require-route.patch. Append the probe shown below and record stage IDs immediately after Unison and High-pass in sj6_run_effects. Compile the donor Juno DSP and overlay runtime as shared C, with X0X_DB, X0X_HOST, X0X_SUPERJ6_FACTORY, X0X_SUPERJ6_BUILTIN_INIT, X0X_SJ6_SOURCE_MIX_WITNESS, X0X_SJ6_NATIVE_UNISON, and X0X_SJ6_LFO1_DELAY_PORT all enabled. Compile variants with DNBOX_SJ6_SOURCE_UNISON_ORDER 0/1 respectively. Run check.py next to both owner-0.so and owner-1.so. No original assets needed to run these synthetic stereo pilots.


### ordered.patch

```diff
--- a/app/original_runtime.c
+++ b/app/original_runtime.c
@@ -1424,6 +1424,17 @@
 #endif
                             )
 {
+#if DNBOX_SJ6_SOURCE_UNISON_ORDER
+            /* Actual current app route: oscillator mixer -> Unison -> HP.
+             * Leave other effect kernels/branches unchanged in this arm. */
+            #if X0X_SJ6_AUX_LFO_NATIVE
+            if(mod && (mod[1]>=0.f || mod[2]>=0.f))
+                juno6_unison_process_mod(&p->dsp.juno.synth,left,right,frames,mod[1],mod[2]);
+            else
+#endif
+            juno6_unison_process(&p->dsp.juno.synth,left,right,frames);
+            juno6_sj6_hpf_bus(&p->dsp.juno.synth,left,right,frames);
+#endif
 #if (defined(X0X_HOST) && defined(X0X_SJ6_ALT3_WITNESS)) || X0X_SJ6_ALT3_NATIVE
 #if X0X_SJ6_AUX_LFO_NATIVE
             if(mod && (mod[3]>=0.f || mod[4]>=0.f))
@@ -1454,6 +1465,7 @@
 #if (defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)) || X0X_SJ6_NATIVE_DELAY
             juno6_delay24_process(&p->dsp.juno.synth,left,right,frames);
 #endif
+#if !DNBOX_SJ6_SOURCE_UNISON_ORDER
 #if X0X_SJ6_NATIVE_UNISON
             #if X0X_SJ6_AUX_LFO_NATIVE
             if(mod && (mod[1]>=0.f || mod[2]>=0.f))
@@ -1462,6 +1474,7 @@
 #endif
             juno6_unison_process(&p->dsp.juno.synth,left,right,frames);
 #endif
+#endif
 #if X0X_SJ6_NATIVE_ROOM
             sj6_crush_process(p,left,right,frames);
             sj6_room_process(&p->dsp.juno.room,left,right,frames);
@@ -1583,7 +1596,11 @@
             s->aux_fx_schedule=&sched;
             break;
         }
+#if DNBOX_SJ6_SOURCE_UNISON_ORDER
+        ok=juno6_sj6_render_osc_bus(s,left,right,frames);
+#else
         ok=juno6_render_stereo(s,left,right,frames);
+#endif
         s->aux_fx_schedule=0;
         if(ok){
             if(!sched.count)sj6_run_effects(p,left,right,frames,0);
@@ -1596,7 +1613,11 @@
                 }
             }
 #else
+#if DNBOX_SJ6_SOURCE_UNISON_ORDER
+        ok=juno6_sj6_render_osc_bus(&p->dsp.juno.synth,left,right,frames);
+#else
         ok=juno6_render_stereo(&p->dsp.juno.synth,left,right,frames);
+#endif
         if(ok){
             sj6_run_effects(p,left,right,frames);
 #endif
```

### require-route.patch

```diff
--- a/firmware/src/app/original_runtime.c
+++ b/firmware/src/app/original_runtime.c
@@ -1600,6 +1600,15 @@
     p->sj6_source_pending=0;
     if(p->kind!=ORIGINAL_SOUND_JUNO||!left||!right||left==right||!frames||frames>256u||
        b->frames!=frames||!(b->trim>=0.f&&b->trim<=1.f))return 0;
+#if !DNBOX_SJ6_SOURCE_UNISON_ORDER
+    /* This inlet skips juno6_render_stereo, which otherwise owns HP.
+     * Refuse instead of silently dropping it or changing Unison order. */
+    const juno6_t *owner=&p->dsp.juno.synth;
+    if(owner->hpf>0.f || owner->hpf < -1.f)return 0;
+#if X0X_SJ6_NATIVE_UNISON
+    if(owner->unison.taps)return 0;
+#endif
+#endif
 #if !X0X_SUPERJ6_BUILTIN_INIT
     if(!p->dsp.juno.synth.typed)return 0;
 #endif
```

### check.py

```python
from pathlib import Path
import ctypes as C,numpy as np,json,os
D=Path(__file__).parent
F=C.c_float; P=C.POINTER(F)
class Block(C.Structure):
 _fields_=[('left',P*4),('right',P*4),('volume',F*4),('pan',F*4),('trim',F),('frames',C.c_uint)]
fn=[]
for mode in (0,1):
 lib=C.CDLL(str(D/f'owner-{mode}.so'),mode=os.RTLD_LAZY)
 f=lib.sj6_probe_compose; f.argtypes=[C.POINTER(Block),P,P,C.c_uint,C.c_uint,P,P,C.POINTER(C.c_uint)];f.restype=C.c_int
 fn.append(f)
def ap(x):return x.ctypes.data_as(P)
m=256;t=np.arange(m,dtype=float)/44100
sig=(.36+.14*np.cos(t*2*np.pi*330)+.025*np.sin(t*2*np.pi*6900)).astype('f4')
z=np.zeros_like(sig);ns=-sig
rows=[]
for name,right in [('left-only',z),('anti-phase',ns)]:
 for unison in (0,1):
  b=Block((P*4)(ap(sig),ap(z),ap(z),ap(z)),(P*4)(ap(right),ap(z),ap(z),ap(z)),
          (F*4)(1,1,1,1),(F*4)(),.8670601844787598,m)
  for flag,f in enumerate(fn):
   x=np.full((2,m),23,dtype='f4');ref=np.empty_like(x);trace=C.c_uint(0)
   ok=f(C.byref(b),*map(ap,x),m,unison,*map(ap,ref),C.byref(trace))
   if not flag:
    assert ok==-2 and np.all(x==23) and trace.value==9
    rows.append(dict(pilot=name,unison=bool(unison),ordered_flag=False,refused_unchanged=True))
    continue
   assert ok==1
   exact=x.tobytes()==ref.tobytes()
   assert trace.value==(0x912 if flag else 9)
   assert exact==bool(flag)
   assert np.max(np.abs(x[0]))>.01
   if name=='left-only':assert not x[1].any()
   else:assert np.max(np.abs(x[1]))>.01 and np.max(np.abs(x[0]-x[1]))>.03
   rows.append(dict(pilot=name,unison=bool(unison),ordered_flag=bool(flag),trace=hex(trace.value),
                   exact=exact,peak_delta=float(np.abs(x-ref).max()),
                   side_peak=float(np.abs(x[0]-x[1]).max())))
print(json.dumps(dict(cases=rows,scope='Host runtime stage reach and signed stereo topology; exact C kernel composition not original installed response'),indent=2))
```

### before-guard.json

```json
{
  "cases": [
    {
      "pilot": "left-only",
      "unison": false,
      "ordered_flag": false,
      "trace": "0x9",
      "exact": false,
      "peak_delta": 0.3764503300189972,
      "side_peak": 0.4541011154651642
    },
    {
      "pilot": "left-only",
      "unison": false,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.42496296763420105
    },
    {
      "pilot": "left-only",
      "unison": true,
      "ordered_flag": false,
      "trace": "0x9",
      "exact": false,
      "peak_delta": 0.24653606116771698,
      "side_peak": 0.29785072803497314
    },
    {
      "pilot": "left-only",
      "unison": true,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.2676127552986145
    },
    {
      "pilot": "anti-phase",
      "unison": false,
      "ordered_flag": false,
      "trace": "0x9",
      "exact": false,
      "peak_delta": 0.3764503300189972,
      "side_peak": 0.9082022309303284
    },
    {
      "pilot": "anti-phase",
      "unison": false,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.8499259352684021
    },
    {
      "pilot": "anti-phase",
      "unison": true,
      "ordered_flag": false,
      "trace": "0x9",
      "exact": false,
      "peak_delta": 0.24653606116771698,
      "side_peak": 0.5870339870452881
    },
    {
      "pilot": "anti-phase",
      "unison": true,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.5351038575172424
    }
  ],
  "scope": "Host runtime stage reach and signed stereo topology; exact C kernel composition not original installed response"
}
```

### result.json

```json
{
  "cases": [
    {
      "pilot": "left-only",
      "unison": false,
      "ordered_flag": false,
      "refused_unchanged": true
    },
    {
      "pilot": "left-only",
      "unison": false,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.42496296763420105
    },
    {
      "pilot": "left-only",
      "unison": true,
      "ordered_flag": false,
      "refused_unchanged": true
    },
    {
      "pilot": "left-only",
      "unison": true,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.2676127552986145
    },
    {
      "pilot": "anti-phase",
      "unison": false,
      "ordered_flag": false,
      "refused_unchanged": true
    },
    {
      "pilot": "anti-phase",
      "unison": false,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.8499259352684021
    },
    {
      "pilot": "anti-phase",
      "unison": true,
      "ordered_flag": false,
      "refused_unchanged": true
    },
    {
      "pilot": "anti-phase",
      "unison": true,
      "ordered_flag": true,
      "trace": "0x912",
      "exact": true,
      "peak_delta": 0.0,
      "side_peak": 0.5351038575172424
    }
  ],
  "scope": "Host runtime stage reach and signed stereo topology; exact C kernel composition not original installed response"
}
```

### Probe code

```c
int sj6_probe_compose(const sj6_source_block *b,float *l,float *r,unsigned n,unsigned unison,
                       float *rl,float *rr,unsigned *trace)
{
    original_runtime_part p={0},ref={0};
    original_runtime_workspace w;
    int16_t rings[2][2][SJ6_UNISON_HISTORY]={0};
    p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
#if !X0X_SUPERJ6_BUILTIN_INIT
    p.dsp.juno.synth.typed=1;
#endif
    p.dsp.juno.synth.core.sample_rate=44100.f;
    p.dsp.juno.synth.master_gain=1.f;
    p.dsp.juno.synth.hpf=.32f;
    p.dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
    const uint8_t code[]={unison,72,96,1};
    ref=p;
    juno6_unison_config(&p.dsp.juno.synth,code,rings[0]);
    juno6_unison_config(&ref.dsp.juno.synth,code,rings[1]);
    for (unsigned i=0;i<n;i++){
      float a=b->left[0][i],z=b->right[0][i];
      for (unsigned k=1;k<4;k++){a+=b->left[k][i];z+=b->right[k][i];}
      rl[i]=a*b->trim;rr[i]=z*b->trim;
    }
    juno6_unison_process(&ref.dsp.juno.synth,rl,rr,n);
#if DNBOX_SJ6_SOURCE_UNISON_ORDER
    juno6_sj6_hpf_bus(&ref.dsp.juno.synth,rl,rr,n);
#else
    /* Known direct HP projection for fail control only. */
    float hp=1.f/(1.f+6.2831853071795864769f*(15.f+2000.f*.32f*.32f)/44100.f);
    float li=0,ri=0,ls=0,rs=0;
    for(unsigned j=0;j<n;j++){float x=rl[j],y=rr[j];
       ls=hp*(ls+x-li);rs=hp*(rs+y-ri);li=x;ri=y;rl[j]=ls;rr[j]=rs;}
#endif
    p.sj6_source_pending=b;
    route_trace=9u;
    int ok=render_backend(&p,&w,l,r,n);
    *trace=route_trace;
    return ok;
}
```
