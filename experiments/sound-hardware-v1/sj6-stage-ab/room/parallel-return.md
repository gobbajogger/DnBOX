# SuperJ6 Type II Room: retain the parallel dry path

Diagnostic patch only. The reached current original pairs an independent dry branch with an additive wet return. Incumbent compact Room instead interpolates between dry and wet and at Mix=127 therefore mutes dry until its first reflection. Closing the send also clears the wet history. This patch leaves the compact room network, wet scale and Type I behavior alone, while separating Type II input send, feedback lifetime and the summing operation. It is not an authentic Room kernel, full effect-chain match, or timing pass. Wet scaling and decay/retirement under continuously closed send are still open.

The same signed 24,576-frame stereo input proves 992 dry-prefix words no longer go through the Type II gain reduction before the first wet frame (496). 49,152 Type I output words are bit-exact both steady and with control toggles, and 49,152 Type II cold bypass words are exact. With held dry across two close/reopen sequences (boundaries 6143/9971/14211/16607), 49,152 output words match the expected independent dry + aged wet path exactly; both closed windows have nonzero wet RMS. Existing Type II wet kernel is unchanged.

On the exact reconstructed diagnostic used by c24f2f2f, native JieLi links pass Elite/Pro with **756/768 B** XIP headroom (32 B less than the incumbent per board) and unchanged **2,588 B** usable pool after the reserve. Images are labeled never-install. No native emulator runtime was run on this patch.

## Reproduce

Follow the c24f2f2f joint diagnostic reconstruction. Set DONOR to the resulting tree before or after applying this patch (the bridge references the file in that tree). Save a prepatch copy of the incumbent source as incumbent.inc first. The complete patch and test sources are below. Save the outputs adjacent to the Python script.

```sh
patch -d "$DONOR" -p1 < parallel-return.patch
sed 's#"../dsp/fastmath.h"#"dsp/fastmath.h"#' incumbent.inc > old.inc
sed 's#"dsp/sj6_native_room.inc"#"old.inc"#' bridge.c > old-bridge.c
for v in old-bridge bridge; do
 cc -shared -fPIC -std=c11 -O2 -ffp-contract=off \
  -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 \
  -DX0X_SJ6_NATIVE_ROOM=1 -I"$DONOR/firmware/src" \
  -I"$DONOR/experiments/original_synth" "$v.c" -lm -o "$v.so"
done
python check.py
```

### parallel-return.patch

```diff
--- a/firmware/src/dsp/sj6_native_room.inc
+++ b/firmware/src/dsp/sj6_native_room.inc
@@ -49,11 +49,11 @@
 {
     if(!p||p->kind!=ORIGINAL_SOUND_JUNO||p->status!=ORIGINAL_RUNTIME_OK)return;
     sj6_room_t *s=&p->dsp.juno.room;
-    if(s->enable!=word[SJ6_ROOM_ENABLE]||s->type!=word[SJ6_ROOM_TYPE])sj6_room_reset(s);
+    if(s->type!=word[SJ6_ROOM_TYPE]||(!word[SJ6_ROOM_TYPE]&&s->enable!=word[SJ6_ROOM_ENABLE])){sj6_room_reset(s);s->armed=0;}
     if(s->widen!=word[SJ6_ROOM_WIDEN]){
         sj6_fatten_reset(s);
     }
-    s->armed=1;
+    s->armed=(uint8_t)(word[SJ6_ROOM_TYPE] ? (s->armed==2||word[SJ6_ROOM_ENABLE]?2:1) : 1);
     s->enable=word[SJ6_ROOM_ENABLE];s->type=word[SJ6_ROOM_TYPE];
     s->widen=word[SJ6_ROOM_WIDEN];
     s->mix=(float)word[SJ6_ROOM_MIX]*(1.f/127.f);
@@ -122,16 +122,21 @@
 }
 static void sj6_room_process(sj6_room_t *s,float *left,float *right,unsigned frames)
 {
-    if(!s->armed||(!s->enable&&!s->widen&&s->depth==0.f))return;
+    if(!s->armed||(!s->enable&&s->armed!=2&&!s->widen&&s->depth==0.f))return;
     for(unsigned i=0;i<frames;++i){
         float l=left[i],r=right[i];
-        if(s->enable){
-            s->acc[0]+=l;s->acc[1]+=r;
+        if(s->enable||s->armed==2){
+            if(s->enable){s->acc[0]+=l;s->acc[1]+=r;}
             if(++s->tick==SJ6_ROOM_STRIDE){s->tick=0;sj6_room_tick(s);}
             if(s->mix>0.f){
                 float t=(float)s->tick*(1.f/SJ6_ROOM_STRIDE);
-                l+=(s->last[0]+t*(s->wet[0]-s->last[0])-l)*s->mix;
-                r+=(s->last[1]+t*(s->wet[1]-s->last[1])-r)*s->mix;
+                /* Type II is a parallel return: retain dry even before the
+                 * first reflection or while the inlet is closed. This
+                 * diagnostic leaves the wet scale and kernel unchanged. */
+                const float wl=s->last[0]+t*(s->wet[0]-s->last[0]);
+                const float wr=s->last[1]+t*(s->wet[1]-s->last[1]);
+                l+=(s->type?wl:wl-l)*s->mix;
+                r+=(s->type?wr:wr-r)*s->mix;
             }
         }
         if(s->widen)r=sj6_fatten_tick(s,r);
```

### bridge.c

```c
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "dsp/juno6.h"
#include "app/original_runtime.h"
#include "dsp/sj6_native_room.inc"
float render_room(unsigned type,unsigned size,unsigned enabled,float *l,float *r,unsigned n) {
    original_runtime_part p={0};
    p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_ENABLE]=enabled,[SJ6_ROOM_TYPE]=type,
      [SJ6_ROOM_SIZE]=size,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0
    };
    original_runtime_sj6_room(&p,code);sj6_room_process(&p.dsp.juno.room,l,r,n);
    return p.dsp.juno.room.fb;
}
float render_room_close(unsigned close,float *l,float *r,unsigned n) {
    original_runtime_part p={0};
    p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_ENABLE]=1,[SJ6_ROOM_TYPE]=1,
      [SJ6_ROOM_SIZE]=80,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0
    };
    if(close>n)return -1.f;
    original_runtime_sj6_room(&p,code);
    sj6_room_process(&p.dsp.juno.room,l,r,close);
    code[SJ6_ROOM_ENABLE]=0;
    original_runtime_sj6_room(&p,code);
    sj6_room_process(&p.dsp.juno.room,l+close,r+close,n-close);
    return p.dsp.juno.room.fb;
}
int render_room_toggle(unsigned toggle,float *l,float *r,unsigned n) {
    static const unsigned boundary[]={0,6143,9971,14211,16607,24576};
    original_runtime_part p={0};p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_ENABLE]=1,[SJ6_ROOM_TYPE]=1,
      [SJ6_ROOM_SIZE]=80,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0
    };
    if(toggle>1u || n!=boundary[5])return 0;
    for(unsigned i=0;i<5;i++){
      code[SJ6_ROOM_ENABLE]=(uint8_t)(toggle?!(i&1u):1u);
      original_runtime_sj6_room(&p,code);
      unsigned j=boundary[i];
      sj6_room_process(&p.dsp.juno.room,l+j,r+j,boundary[i+1]-j);
    }
    return 1;
}
int render_room_dynamic(unsigned kind,unsigned hold_off,float *l,float *r,unsigned n) {
    static const unsigned boundary[]={0,6143,9971,14211,16607,24576};
    original_runtime_part p={0};p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_SIZE]=80,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0,[SJ6_ROOM_TYPE]=kind
    };
    if(n!=boundary[5])return 0;
    for(unsigned i=0;i<5;i++){
      code[SJ6_ROOM_ENABLE]=(uint8_t)(hold_off?!(i&1u):1u);
      original_runtime_sj6_room(&p,code);
      unsigned j=boundary[i];
      sj6_room_process(&p.dsp.juno.room,l+j,r+j,boundary[i+1]-j);
    }
    return 1;
}
```

### check.py

```python
import ctypes as C
import numpy as np
import json, hashlib
from pathlib import Path
p=Path(__file__).parent
o,n=(C.CDLL(str(p/f'{k}.so')) for k in ('old-bridge','bridge'))
a=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
l=24576
i=np.arange(l,dtype='i8')
x=np.stack(((i*3097%65521-32760)/131072.,(i*4577%65521-32760)/131072.)).astype('f4')
x[:,i%13==5]=0
x[:,0]=(.25,-.125)
def sha(x): return hashlib.sha256(x.astype('<f4').tobytes()).hexdigest()
def static(lib,t,e):
 f=lib.render_room;f.argtypes=[C.c_uint,C.c_uint,C.c_uint,a,a,C.c_uint];f.restype=C.c_float
 k=x.copy();r=f(t,80,e,*k,l)
 assert r==0.7779527902603149
 return k
dry=static(n,1,0); assert sha(dry)==sha(x)
one=static(n,0,1); assert sha(one)==sha(static(o,0,1))
wet=static(o,1,1); y=static(n,1,1)
first=int(np.flatnonzero(np.any(wet,axis=0))[0])
assert sha(y[:,:first])==sha(x[:,:first])
assert not np.any(wet[:,:first])
# Existing mix=1 evaluates dry+(wet-dry); allow its float-rounding.
e=y.astype('f8')-x-wet
assert np.max(np.abs(e)) < 5e-8
print(json.dumps(dict(first_wet_frame=first, dry_words_restored=2*first, type_I_words_exact=2*l,
                       type_I_signed_sha=sha(one), candidate_signed_sha=sha(y), dry_off_words_exact=2*l,
                       parallel_sum_max_error=float(np.max(np.abs(e)))),indent=2))
def dynamic(lib,t,toggle,k):
 f=lib.render_room_dynamic
 f.argtypes=[C.c_uint,C.c_uint,a,a,C.c_uint];f.restype=C.c_int
 k=k.copy(); assert f(t,toggle,*k,l);return k
bounds=[(6143,9971),(14211,16607)]
z=x.copy()
for b,e in bounds: z[:,b:e]=0
r=dynamic(n,1,0,z)
y=dynamic(n,1,1,x)
expected=(x.astype('f8')+r-z).astype('f4')
assert np.max(np.abs(expected-y))<5e-8
assert sha(dynamic(n,0,1,x))==sha(dynamic(o,0,1,x))
assert all(np.linalg.norm(r[:,b:e])>0 for b,e in bounds)
print(json.dumps(dict(dynamic_words=2*l, dynamic_parallel_error=float(np.max(np.abs(expected-y))),
                       type_I_toggle_signed_sha=sha(dynamic(n,0,1,x)),type_II_toggle_signed_sha=sha(y),
                       closed_return_rms=[float(np.sqrt(np.mean(r[:,b:e]**2))) for b,e in bounds]),indent=2))
```

### result.json

```text
{
  "first_wet_frame": 496,
  "dry_words_restored": 992,
  "type_I_words_exact": 49152,
  "type_I_signed_sha": "8e8b590af795f1dac45ec2ac8415d926ce93d863d8553700e51c465a63fdef35",
  "candidate_signed_sha": "a330286ef46c6cb7acdeeaf70542639feebbfeda4e66a0a4875bfba75db1db85",
  "dry_off_words_exact": 49152,
  "parallel_sum_max_error": 1.4901161193847656e-08
}
{
  "dynamic_words": 49152,
  "dynamic_parallel_error": 0.0,
  "type_I_toggle_signed_sha": "4c44adde6820bffdaa60dcfed2afd54dafd8f3a3dc062b2b128a198bfadb6b1c",
  "type_II_toggle_signed_sha": "b0f462c3c8d6a99321cb21d2728ec06c835ea62e59cf49b48d79fb9b5c789ee8",
  "closed_return_rms": [
    0.024975428357720375,
    0.028096791356801987
  ]
}
```
