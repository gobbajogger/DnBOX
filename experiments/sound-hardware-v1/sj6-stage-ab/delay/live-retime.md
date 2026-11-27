# Preserve SuperJ6's active echo when a live Delay time edit changes stride

Diagnostic only. The current compact implementation discarded both delay channels' history when a live time edit changed its decimation stride. The candidate reuses the existing in-place coarsening path for longer representable taps and retains the live history for a shorter tap that the current cadence still represents. It does not change steady delay response, introduce a PingPong control, fix 3 kHz aliasing, or claim the original's live tap glide. Sub-cell time edits follow the incumbent to avoid silently clamping requested timing to an old sample cell. The old clock can remain coarse on a warm shortening until a later disarmed edit. The synchronous coarsening cost has not passed an audio deadline test.

Signed stereo host A/B: the pre-edit output matches bit-for-bit (38,226 words per arm). Longer stride retains 128 completed cells, versus zero before; shorter stride preserves all 554 cells versus zero. Both held-tail stimuli go from silence to a nonzero stereo echo. Ultra-short fallback and no-op edits match incumbent for the entire 80,000 words each. The prototype's negative and positive cases use the actual delay include with the isolated probe layout. An independently enumerated in-place chronology test sampled 2,016 layouts without read/write overlap errors.

## Rerun

Extract the patch, C bridge, and Python test below into the same directory. Copy the incumbent juno6_delay_native.inc from the pinned joint-recompose source into old/ and new/. Apply the patch to the isolated source before copying it to new/juno6_delay_native.inc. Run `cc -std=c11 -O2 -fPIC -shared -Wall -Wextra -Werror -ffp-contract=off -Iold probe.c -lm -o old.so` and likewise with new. Run `python verify.py`.


### retime.patch

```diff
--- a/firmware/src/dsp/juno6_delay_native.inc
+++ b/firmware/src/dsp/juno6_delay_native.inc
@@ -62,6 +62,8 @@
     return native_delay_frames(s,lo,bpm,s->delay24.sync);
 }
 
+#endif
+
 /* In-place coarsening aligned on the newest complete old cell. Write
  * indices trail the already-read source indices, including across wrap. */
 static void native_delay_route_coarsen(juno6_t *s,unsigned wanted)
@@ -91,6 +93,7 @@
     /* The existing partial bin stays pending; phase < old <= wanted,
        and both channels already hold the exact uncommitted time sum. */
 }
+#if X0X_SJ6_AUX_LFO_NATIVE
 void juno6_delay_native_route_refresh(juno6_t *s,float bpm)
 {
     if(!s->delay24.tap || !s->delay24.stride || !(bpm>0.f))return;
@@ -128,7 +131,15 @@
         (route==0.f && stride>1u && frames<(float)((SJ6_NATIVE_DELAY_CAP/4u)*stride))) {
         stride=1u;
         while (window>(float)((SJ6_NATIVE_DELAY_CAP-2u)*stride)) stride<<=1;
-        if (stride!=s->delay24.stride) {
+        const unsigned old=s->delay24.stride;
+        if(stride!=old && old && s->delay24.filled &&
+           (stride>old || frames>=2.f*(float)old) &&
+           (s->delay24.wet!=0.f || s->delay24.wet_target!=0.f ||
+            s->delay24.fb!=0.f || s->delay24.fb_target!=0.f)){
+            if(stride>old)native_delay_route_coarsen(s,stride);
+            /* A still-representable shorter tap can use the live clock.
+             * Only a dry or sub-cell edit may discard and re-arm it. */
+        } else if (stride!=old) {
             s->delay24.stride=(uint16_t)stride;
             s->delay24.filled=s->delay24.phase=s->delay24.pos=0;
             s->delay24.sum[0]=s->delay24.sum[1]=0.f;
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
    uint8_t tick,mix,clock_refused,sync,enabled;
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
```

### verify.py

```python
from pathlib import Path
import ctypes as C,numpy as np,json
p=Path(__file__).parent
a=np.ctypeslib.ndpointer(dtype='float32',ndim=1,flags='C_CONTIGUOUS')
f=[C.CDLL(str(p/f'{k}.so')).retime for k in ('old','new')]
for g in f:
 g.argtypes=[C.c_uint,C.c_uint,a,a,C.c_uint,C.c_uint,C.POINTER(C.c_uint)];g.restype=None
n=40000;cut=19113
i=np.arange(cut)
x=np.zeros((2,n),dtype='f4')
x[:,:cut]=np.stack([.13*np.sin(i*.1),.08*np.cos(i*.27)])
rows=[]
for t in [(8,64),(64,8),(64,0),(64,64)]:
 r=[]; y=[]
 for g in f:
  z=x.copy();o=(C.c_uint*8)()
  g(*t,*z,n,cut,o);r.append(list(o));y.append(z)
 assert y[0][:,:cut].tobytes()==y[1][:,:cut].tobytes()
 rms=[float(np.linalg.norm(k[:,cut:].astype('f8'))/np.sqrt(2*(n-cut))) for k in y]
 old,b=r
 if t in [(8,64),(64,8)]:
  assert old[5]==0 and old[6]==0 and rms[0]==0. and rms[1]>0.
  assert b[5]==b[1]//(b[4]//b[0]) and b[6:8]==b[2:4]
 else:
  assert b==old and y[0].tobytes()==y[1].tobytes()
 rows.append(dict(detents=t,old_state=old,new_state=b,tail_rms=rms,exact_pre_edit_words=2*cut))
print(json.dumps(rows,indent=2))
```

### result.json

```json
[
  {
    "detents": [
      8,
      64
    ],
    "old_state": [
      8,
      1024,
      1,
      341,
      64,
      0,
      0,
      0
    ],
    "new_state": [
      8,
      1024,
      1,
      341,
      64,
      128,
      1,
      341
    ],
    "tail_rms": [
      0.0,
      0.0008688100519509815
    ],
    "exact_pre_edit_words": 38226
  },
  {
    "detents": [
      64,
      8
    ],
    "old_state": [
      64,
      554,
      41,
      554,
      8,
      0,
      0,
      0
    ],
    "new_state": [
      64,
      554,
      41,
      554,
      64,
      554,
      41,
      554
    ],
    "tail_rms": [
      0.0,
      0.0013527132635410468
    ],
    "exact_pre_edit_words": 38226
  },
  {
    "detents": [
      64,
      0
    ],
    "old_state": [
      64,
      554,
      41,
      554,
      1,
      0,
      0,
      0
    ],
    "new_state": [
      64,
      554,
      41,
      554,
      1,
      0,
      0,
      0
    ],
    "tail_rms": [
      0.0,
      0.0
    ],
    "exact_pre_edit_words": 38226
  },
  {
    "detents": [
      64,
      64
    ],
    "old_state": [
      64,
      554,
      41,
      554,
      64,
      554,
      41,
      554
    ],
    "new_state": [
      64,
      554,
      41,
      554,
      64,
      554,
      41,
      554
    ],
    "tail_rms": [
      0.0014805765268205461,
      0.0014805765268205461
    ],
    "exact_pre_edit_words": 38226
  }
]
```

### Native diagnostic link outputs

```text
guarded elite XIP 551948 free 724 pool usable 2588 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000001000000d8ef00007def59ef0281a3cadd52e1126c4e48f0 ELF d31e4dda75eeee2f033381565029c4fde6c4e6c4b686e25c8ff7b4fbb1c8385f
guarded pro XIP 551936 free 736 pool usable 2588 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000002000000d8ef00007def59efb401f93bcc1013a628f647c7 ELF d1d849ce6c936aac4c793f933876bddb2729ff5e98e8a47c5245fc62dc1e0eea
```
