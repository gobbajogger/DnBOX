# Do not leave SuperJ6 Delay at its long-history rate after shortening

An actual C reproduction confirms the warm long→short risk left by the initial retime diagnostic: a 3-second to a quarter-second live Sync edit preserved the old tail but retained stride 256, and new 1/3/6 kHz echoes fell well below even cold quarter-second performance. This is not a theoretical note; same live controls, signed tail-only baseline and fresh signed windowed bursts were scored against cold C and the Build9-validated full-rate reference. Existing steady 3 and 6 kHz loss of the compact codec is a separate remaining difference.

The new diagnostic refines the same ring at the first complete coarse cell after the live time ramp has settled. It duplicates only the reachable signed old cells in place, oldest to newest, so no copy reads an already-overwritten cell. It does not invent high frequencies in the historic tail. All new cells then run at the same fine cadence as cold quarter-second. One unused trailing byte stores the pending power-of-two stride request; all measured linked pool budgets are unchanged. Routed LFO time owns its own maximum window and clears pending refinement. The sample-floor fallback is unchanged.

- Actual 120 BPM, 44.1 kHz, 3 s → 250 ms: formerly 256→256; candidate 256→16 after settle.
- First fresh 1 kHz true carrier: formerly **−89.72 dB**, now **−5.8883 dB** relative to the original isolated full-rate reference, equal to cold compact. At 3/6 kHz, actual signed first-echo windows likewise match cold carrier/alias results, still **−65.55/−65.98 dB** true carrier; this confirms that new history returned to cold behavior, not an overall original delay match.
- Tail-only control stays nonzero, with exact prior waveform before the conversion window (~9800 frames of the initial tail). Right channel does not receive the new left-only tone. No full reference live time-glide claim.
- Actual PI32 isolated conversion: 640 planted wrapped/filled/cell-ratio cases, zero comparisons diverged; worst 2620 ticks of the 24 MHz timer = 109.17 µs at 360 MHz, serialized diagnostic only. This is a one-time cost and still needs active full-audio deadline qualification.
- Paired joined-with-clock diagnostic links: Elite/Pro APP free 268/280 B, **+256 B executable** compared with the pre-refinement joined+clock candidate; usable pool unchanged **3236 B**. Not production, not zero dropout, no active paired workload replay run with this patch.

The overnight research hypothesis was accepted only after actual signed C reproduction and measured cold/warm comparison. No duplicate summary of previous timing attribution.

## Restore / replay

Use the joined baseline with the clock correction. Old/new includes refer to pre/post refinement on that same tree. The test's probe.c uses a host-only matching struct with the extra trailing byte; native firmware change is the independent header diff. Compile the two with `cc -std=c11 -O2 -fPIC -shared -Wall -Wextra -Werror -ffp-contract=off -Iold probe.c -lm -o old.so` and repeat for new. For `ref.so`, compile the source-backed `sj6_stereo_delay.c` as the original 250ms reference then run `python verify.py`. The native leaf bench uses the same `bench.ld` and `start.S` as the saved ladder raw payload, one 900 ms strict emulator run, output five LE words at dump offset 0x10000.


### refine.patch

```diff
--- a/firmware/src/dsp/juno6_delay_native.inc
+++ b/firmware/src/dsp/juno6_delay_native.inc
@@ -96,11 +96,43 @@
     /* The existing partial bin stays pending; phase < old <= wanted,
        and both channels already hold the exact uncommitted time sum. */
 }
+/* Cold high-rate data cannot be recreated. Convert only the reachable
+ * recent history and resume writing new high-rate data after the ramp. */
+static void native_delay_refine_completed(juno6_t *s)
+{
+    const unsigned wanted=1u<<(s->delay24.refine-1u);
+    const unsigned old=s->delay24.stride;
+    s->delay24.refine=0;
+    if(wanted>=old)return;
+    const unsigned ratio=old/wanted;
+    unsigned filled=s->delay24.filled*ratio;
+    if(filled>SJ6_NATIVE_DELAY_CAP)filled=SJ6_NATIVE_DELAY_CAP;
+    enum { MASK=SJ6_NATIVE_DELAY_CAP-1u };
+    unsigned pos=s->delay24.pos;
+    if(filled){
+        unsigned from_age=(filled-1u)/ratio+1u;
+        unsigned group=((filled-1u)&(ratio-1u))+1u;
+        for(unsigned age=filled;age;--age){
+            const unsigned from=((pos-from_age)&MASK)*2u;
+            const unsigned to=((pos-age)&MASK)*2u;
+            const int16_t l=s->delay24.tap[from],r=s->delay24.tap[from+1u];
+            s->delay24.tap[to]=l;s->delay24.tap[to+1u]=r;
+            if(--group==0u){--from_age;group=ratio;}
+        }
+    }
+    s->delay24.filled=(uint16_t)filled;
+    s->delay24.stride=(uint16_t)wanted;
+    s->delay24.inv_stride=1.f/(float)wanted;
+    s->delay24.current*=(float)ratio;
+    s->delay24.target*=(float)ratio;
+    s->delay24.step[0]*=(float)ratio;
+}
 #if X0X_SJ6_AUX_LFO_NATIVE
 void juno6_delay_native_route_refresh(juno6_t *s,float bpm)
 {
     if(!s->delay24.tap || !s->delay24.stride || !(bpm>0.f))return;
     const float window=native_delay_route_window(s,bpm);
+    if(window>0.f)s->delay24.refine=0;
     unsigned stride=s->delay24.stride;
     while(window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride))stride<<=1;
     if(stride!=s->delay24.stride)native_delay_route_coarsen(s,stride);
@@ -114,6 +146,7 @@
     s->delay24.clock_refused=!(frames>0.f);
     s->delay24.sync=(uint8_t)!!(b[0]&2u);
     s->delay24.enabled=b[0]&1u;
+    s->delay24.refine=0;
     if (!(frames>0.f)) return;
     /* Power of two bins permit stable repeats, and hysteresis minimizes the
      * only non-transplantable edit: switching the history sample rate. */
@@ -140,6 +173,11 @@
            (s->delay24.wet!=0.f || s->delay24.wet_target!=0.f ||
             s->delay24.fb!=0.f || s->delay24.fb_target!=0.f)){
             if(stride>old)native_delay_route_coarsen(s,stride);
+            else if(route==0.f){
+                uint8_t refin=1;
+                for(unsigned v=stride;v>1u;v>>=1)++refin;
+                s->delay24.refine=refin;
+            }
             /* A still-representable shorter tap can use the live clock.
              * Only a dry or sub-cell edit may discard and re-arm it. */
         } else if (stride!=old) {
@@ -218,6 +256,8 @@
             s->delay24.phase=0;
             s->delay24.pos=(uint16_t)((pos+1u)&MASK);
             if (s->delay24.filled<SJ6_NATIVE_DELAY_CAP) ++s->delay24.filled;
+            if(s->delay24.refine && !s->delay24.ramp[0])
+                native_delay_refine_completed(s);
         }
         if (!s->delay24.clock_refused && s->delay24.wet!=0.f) {
             float dry=1.f-s->delay24.wet;
```

### refine-header.patch

```diff
--- a/firmware/src/dsp/juno6.h
+++ b/firmware/src/dsp/juno6.h
@@ -149,7 +149,7 @@
     float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];
     uint16_t ramp[3];
     uint16_t stride,phase,pos,filled;
-    uint8_t tick,mix,clock_refused,sync,enabled;
+    uint8_t tick,mix,clock_refused,sync,enabled,refine;
  } delay24;
 #endif
 #if X0X_SJ6_LFO1_DELAY_PORT
```

### probe.c

```c
/* Isolate the candidate's actual delay include without booting the firmware.
 * Layout fields are pinned to juno6.h in the source used for comparison. */
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#define X0X_SJ6_NATIVE_DELAY 1
#define X0X_SJ6_AUX_LFO_NATIVE 0
#define fm_powf powf
enum { SJ6_NATIVE_DELAY_CAP=1024 };
typedef struct {
  int typed;
  float sample_rate,lfo_tempo;
  struct {
    int16_t *tap;
    float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];
    uint16_t ramp[3],stride,phase,pos,filled;
    uint8_t tick,mix,clock_refused,sync,enabled,refine;
  } delay24;
} juno6_t;
#include "juno6_delay_native.inc"

/* Steady controls: finish the target's real startup ramp before injecting
 * a test tone. The target's start/stop or UI mapper is not under test. */
unsigned compact(unsigned raw,unsigned fb,unsigned mx,float *l,float *r,unsigned n) {
    int16_t storage[2*SJ6_NATIVE_DELAY_CAP]={0};
    juno6_t s={.sample_rate=44100,.typed=1};
    juno6_delay_native_arm(&s,storage);
    const uint8_t ctl[]={1,raw,fb,mx};
    juno6_delay_native_config(&s,ctl,120.f);
    float zero_l[128]={0},zero_r[128]={0};
    unsigned stride=s.delay24.stride;
    for(unsigned i=0;i<128;i++)juno6_delay24_process(&s,zero_l,zero_r,128);
    juno6_delay24_process(&s,l,r,n);
    return stride;
}

/* Same signed input and native ramp, live time edit; return the actual clock
 * and chronological state immediately before and after, and render zeros. */
void retime(unsigned first,unsigned then,float *l,float *r,unsigned n,
            unsigned cut,unsigned *out)
{
    int16_t storage[2*SJ6_NATIVE_DELAY_CAP]={0};
    juno6_t s={.sample_rate=44100,.typed=1};
    juno6_delay_native_arm(&s,storage);
    uint8_t ctl[]={1,first,64,127};
    juno6_delay_native_config(&s,ctl,120.f);
    float zero_l[128]={0},zero_r[128]={0};
    for(unsigned i=0;i<128;i++){
        for(unsigned k=0;k<128;k++)zero_l[k]=zero_r[k]=0.f;
        juno6_delay24_process(&s,zero_l,zero_r,128);
    }
    juno6_delay24_process(&s,l,r,cut);
    out[0]=s.delay24.stride;out[1]=s.delay24.filled;
    out[2]=s.delay24.phase;out[3]=s.delay24.pos;
    ctl[1]=(uint8_t)then;
    juno6_delay_native_config(&s,ctl,120.f);
    out[4]=s.delay24.stride;out[5]=s.delay24.filled;
    out[6]=s.delay24.phase;out[7]=s.delay24.pos;
    juno6_delay24_process(&s,l+cut,r+cut,n-cut);
}

float clock_frames(unsigned raw,unsigned sync,float bpm,float sr){
    juno6_t s={.sample_rate=sr,.typed=1};
    return native_delay_frames(&s,raw,bpm,sync);
}
/* 120 BPM Sync raw 127 -> raw 45 (250 ms). Input arrays carry the
 * tail-only or probe stimuli. Match chunk boundaries across old/new. */
void live(unsigned warm,float *l,float *r,unsigned n,unsigned cut,
          unsigned burst,unsigned *out)
{
    int16_t storage[2*SJ6_NATIVE_DELAY_CAP]={0};
    juno6_t s={.sample_rate=44100,.typed=1};
    juno6_delay_native_arm(&s,storage);
    uint8_t ctl[]={3,warm?127:45,64,127};
    juno6_delay_native_config(&s,ctl,120.f);
    float zero_l[128]={0},zero_r[128]={0};
    for(unsigned i=0;i<128;i++){
        for(unsigned j=0;j<128;j++)zero_l[j]=zero_r[j]=0.0f;
        juno6_delay24_process(&s,zero_l,zero_r,128);
    }
    juno6_delay24_process(&s,l,r,cut);out[0]=s.delay24.stride;
    ctl[1]=45;
    juno6_delay_native_config(&s,ctl,120.f);
    out[1]=s.delay24.stride;
    juno6_delay24_process(&s,l+cut,r+cut,burst-cut);out[2]=s.delay24.stride;
    juno6_delay24_process(&s,l+burst,r+burst,n-burst);out[3]=s.delay24.stride;
}
```

### verify.py

```python
import ctypes as C, numpy as np, json
from pathlib import Path
p=Path(__file__).parent
R=44100;cut=R*4+9; burst=cut+R+83; n=burst+R
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
g={}
for mode in ('old','new'):
 f=C.CDLL(str(p/(mode+'.so'))).live
 f.argtypes=[C.c_uint,fp,fp,C.c_uint,C.c_uint,C.c_uint,C.POINTER(C.c_uint)]
 f.restype=None;g[mode]=f
c=C.CDLL(str(p/'ref.so'))
c.sj6_new.argtypes=[C.c_double];c.sj6_new.restype=C.c_void_p
c.sj6_params.argtypes=[C.c_void_p,C.c_double,C.c_float,C.c_float,C.c_int]
c.sj6_process.argtypes=[C.c_void_p,C.c_size_t,fp,fp,fp,fp]
c.sj6_free.argtypes=[C.c_void_p]
def compact(mode,warm,x):
 x=x.copy();o=(C.c_uint*4)();g[mode](warm,*x,n,cut,burst,o);return x,list(o)
s=np.zeros((2,n),'f4')
j=np.arange(8000)
s[0,cut-8000:cut]=.17*np.hanning(8000)*np.sin(j*.003)
s[1,cut-8000:cut]=.12*np.hanning(8000)*np.cos(j*.005)
tails={mode:compact(mode,1,s) for mode in g}
assert tails['old'][1]==[256,256,256,256]
assert tails['new'][1]==[256,256,16,16]
assert tails['old'][0][:,:cut+9800].tobytes()==tails['new'][0][:,:cut+9800].tobytes()
rows=[]
for hz in (1000,3000,6000):
 x=s.copy(); length=4600; t=np.arange(length)
 x[0,burst:burst+length]=.20*np.hanning(length)*np.sin(t*2*np.pi*hz/R)
 z=x-s
 y=np.zeros_like(z)
 k=c.sj6_new(R); assert k; c.sj6_params(k,.25,(.99/127)*64,.1,0);c.sj6_process(k,n,*z,*y);c.sj6_free(k)
 bounds=slice(burst+11025-180,burst+11025+length+180)
 v=y[0,bounds].astype('f8')
 def db(a):
  eps=1e-15
  t=np.arange(a.size); w=np.hanning(a.size)
  carrier=abs(a@(w*np.exp(2j*np.pi*hz/R*t)))
  ref=abs(v@(w*np.exp(2j*np.pi*hz/R*t)))
  fft=abs(np.fft.rfft(a*w))
  bins=np.fft.rfftfreq(a.size,1/R)
  return dict(carrier_dB=float(20*np.log10(max(eps,carrier)/ref)),
     dominant_hz=float(bins[np.argmax(fft)]),
     residual_dBr=float(20*np.log10(max(eps,np.linalg.norm(a-v))/np.linalg.norm(v))))
 row=dict(hz=hz,original=db(v))
 ct,ck=compact('old',0,z);row['cold']=db(ct[0,bounds])
 for mode in g:
  actual,strides=compact(mode,1,x)
  probe=(actual-tails[mode][0])[0,bounds]
  row[mode]=dict(**db(probe),strides=strides,rms_tail=float(np.sqrt(np.mean(tails[mode][0][:,cut:burst]**2))))
  assert np.array_equal(actual[1],tails[mode][0][1])
 assert row['new']['dominant_hz']==row['cold']['dominant_hz'],row
 assert row['new']['rms_tail']>0
 rows.append(row)
print(json.dumps(rows,indent=2))
```

### result.json

```json
[
  {
    "hz": 1000,
    "original": {
      "carrier_dB": 0.0,
      "dominant_hz": 995.8064516129032,
      "residual_dBr": -314.4620966962387
    },
    "cold": {
      "carrier_dB": -5.888304235095348,
      "dominant_hz": 995.8064516129032,
      "residual_dBr": -0.9762013788831754
    },
    "old": {
      "carrier_dB": -89.71648558349196,
      "dominant_hz": 35.564516129032256,
      "residual_dBr": 0.0035129098913514595,
      "strides": [
        256,
        256,
        256,
        256
      ],
      "rms_tail": 0.006174083333462477
    },
    "new": {
      "carrier_dB": -5.888304235095348,
      "dominant_hz": 995.8064516129032,
      "residual_dBr": -0.9762013788831754,
      "strides": [
        256,
        256,
        16,
        16
      ],
      "rms_tail": 0.006190231069922447
    }
  },
  {
    "hz": 3000,
    "original": {
      "carrier_dB": 0.0,
      "dominant_hz": 2996.310483870968,
      "residual_dBr": -314.4620966927168
    },
    "cold": {
      "carrier_dB": -65.55465575245509,
      "dominant_hz": 240.06048387096774,
      "residual_dBr": 0.022237241066857923
    },
    "old": {
      "carrier_dB": -105.21531671159104,
      "dominant_hz": 71.12903225806451,
      "residual_dBr": 0.000491795933545094,
      "strides": [
        256,
        256,
        256,
        256
      ],
      "rms_tail": 0.006174083333462477
    },
    "new": {
      "carrier_dB": -65.55465575245509,
      "dominant_hz": 240.06048387096774,
      "residual_dBr": 0.022237241066857923,
      "strides": [
        256,
        256,
        16,
        16
      ],
      "rms_tail": 0.006190231069922447
    }
  },
  {
    "hz": 6000,
    "original": {
      "carrier_dB": 0.0,
      "dominant_hz": 6001.512096774193,
      "residual_dBr": -314.46209669355835
    },
    "cold": {
      "carrier_dB": -65.98075962696649,
      "dominant_hz": 489.01209677419354,
      "residual_dBr": 0.017955728379757
    },
    "old": {
      "carrier_dB": -138.90385187705863,
      "dominant_hz": 26.673387096774192,
      "residual_dBr": 7.778669231973621e-05,
      "strides": [
        256,
        256,
        256,
        256
      ],
      "rms_tail": 0.006174083333462477
    },
    "new": {
      "carrier_dB": -65.98075962696649,
      "dominant_hz": 489.01209677419354,
      "residual_dBr": 0.017955728379757,
      "strides": [
        256,
        256,
        16,
        16
      ],
      "rms_tail": 0.006190231069922447
    }
  }
]
```

### bench.c

```c
/* Isolate the candidate's actual delay include without booting the firmware.
 * Layout fields are pinned to juno6.h in the source used for comparison. */
#include <stdint.h>
#define X0X_SJ6_NATIVE_DELAY 1
#define X0X_SJ6_AUX_LFO_NATIVE 0
#include "fastmath.h"
enum { SJ6_NATIVE_DELAY_CAP=1024 };
typedef struct {
  int typed;
  float sample_rate,lfo_tempo;
  struct {
    int16_t *tap;
    float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];
    uint16_t ramp[3],stride,phase,pos,filled;
    uint8_t tick,mix,clock_refused,sync,enabled,refine;
  } delay24;
} juno6_t;
#include "juno6_delay_native.inc"
#include "fm1_time.h"
static int16_t h[2*SJ6_NATIVE_DELAY_CAP];
static uint32_t mixes(uint32_t x){return (x*48271u)^(x<<23);}
void bench_start(void){
 volatile uint32_t *o=(void*)0x01c10000;
 uint32_t bad=0,max=0,sum=0,tries=0;
 o[0]=0;fm1_time_init();
 for(unsigned shift=1;shift<=8;shift++)
  for(unsigned start=0;start<1024;start+=113)
   for(unsigned count=1;count<=1024;count=count<17?count+8:2*count-1){
    juno6_t s={.sample_rate=44100,.typed=1};
    unsigned ratio=1u<<shift,n=count*ratio;if(n>1024)n=1024;
    for(unsigned i=0;i<2048;i++)h[i]=(int16_t)mixes(i+start*11u);
    s.delay24.tap=h;s.delay24.stride=(uint16_t)(ratio*16);
    s.delay24.filled=(uint16_t)count;s.delay24.pos=(uint16_t)start;
    s.delay24.refine=5;s.delay24.target=s.delay24.current=1.f;
    uint32_t begin=fm1_ticks();native_delay_refine_completed(&s);
    uint32_t took=fm1_ticks()-begin;if(took>max)max=took;sum+=took;tries++;
    bad+=s.delay24.stride!=16||s.delay24.filled!=n||s.delay24.pos!=start||s.delay24.current!=(float)ratio;
    for(unsigned a=1;a<=n;a++){
        unsigned src=(start-((a-1u)/ratio+1u))&1023,dst=(start-a)&1023;
        bad+=h[2*dst]!=(int16_t)mixes(2*src+start*11u);
        bad+=h[2*dst+1]!=(int16_t)mixes(2*src+1u+start*11u);
    }
   }
 o[1]=max;o[2]=sum;o[3]=bad;o[4]=tries;o[0]=0x52454649;
}
void *memset(void *p,int value,unsigned size){
 volatile unsigned char *d=p;while(size--)*d++=(unsigned char)value;return p;
}
```

### native-resize.json

```json
{"magic": "0x52454649", "max_ticks": 2620, "mean_ticks": 1392.49375, "mismatches": 0, "cases": 640, "max_us": 109.16666666666667}
```

```text
guarded elite XIP 552404 free 268 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000001000000d8ef00007def59efe87524ea5e5670dfb0804576 ELF c78367c5c40f3c91eb5b802a0050e7a73d6cda1e41d01534968b1209deabdc4b
guarded pro XIP 552392 free 280 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000002000000d8ef00007def59efaead065cdfa7323958f16620 ELF a2bb504ca70789dd86fae66202f537130ea943ce2513c80c5928b7ff9eb914ee
```
