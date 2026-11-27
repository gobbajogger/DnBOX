# Synth One: restored signed reference and full 906-preset 25-bit holdout

The first reconstructed Float32 source showed a new phrase mismatch on #327. It was a baseline error, not storage: the older s1u.c translation unit lacks the exact-envelope arithmetic even when the flag is provided. Rebuilding with the pinned release-phase envelope source restored the earlier signed receipts byte for byte for #327 and #366. The code difference is global envelope pole and threshold rounding, not preset repair.

On that corrected host donor the 21-bit full-rate ring still changes two late spectral cases (#379 and #456). 25-bit packed full-rate storage passes the complete **906/906** host sweep after correction, unchanged source preset/scale/notes and four independent delay histories. Worst signed whole render **−47.56405 dBr**, worst tail spectral deviation **1.16702 dB**, 300 candidate renders byte-identical to the earlier host baseline. No fitted patch trim, no lowered sample rate and no native claim. The narrowed actual packed storage is still **2,067,290 B per owner** at 44.1 kHz, so this is a fidelity reference, not a target memory solution.

Source-only proof archive SHA256 9bd12e47284672dbfb4875ee07c389a3e13916b5eb68430396f65d21f9df60f6, per-preset signed evidence SHA256 c3e73f7dcaf5d0c45386da302fee4fad906538e584e96510d055186e9ab7f49a. The archive contains no PCM or preset payload. These gates are declared engineering checks (signed whole ≤−40 dBr, tail shape <3dB), not bit-identical all presets or a listening/hardware acceptance statement. At #379 the strict dimension is sensitive to small tail bands; no relaxation intended.

## Corrected envelope donor difference and full bank summary


### donor-adsr.diff

```diff
--- a/s1u.c
+++ b/s1u.c
@@ -1,3 +1,4 @@
+#include <math.h>
 /* SPDX-License-Identifier: GPL-3.0-only */
 /* Synth One shared units (see s1u.h). The control laws are the compact engine's; moving them here
  * changes no output (checked bit-exact against the inline versions on the device configuration). */
@@ -37,16 +38,20 @@
 #define S1U_TWO_PI 6.28318530718f
 
 /* ---- ADSR ---- */
+#if S1_ENVELOPE_EXACT
+static float pole(float t,float sr){return (float)exp(-1.0/(double)(t*sr));}
+#else
 static float pole(float t, float sr) { return t <= 0.0f ? 0.0f : expf(-1.0f / (t * sr)); }
+#endif
 float s1u_env_step(s1u_env *e, float gate, float atk, float dec, float sus, float rel, float sr)
 {
-    if (e->prev < gate && e->mode != S1U_E_DEC) { e->mode = S1U_E_ATK; e->a = pole(atk * 0.6f, sr); }
+    if (e->prev < gate && e->mode != S1U_E_DEC) { e->mode = S1U_E_ATK; e->a = pole(S1_ENVELOPE_EXACT ? (float)((double)atk*.6) : atk*.6f, sr); }
     else if (e->prev > gate) { e->mode = S1U_E_REL; e->a = pole(rel, sr); }
     e->prev = gate;
     if (e->mode == S1U_E_CLEAR) return 0.0f;
     float x = e->mode == S1U_E_ATK ? gate : gate * sus;
     e->y = (1.0f - e->a) * x + e->a * e->y;
-    if (e->mode == S1U_E_ATK && e->y > 0.99f) { e->mode = S1U_E_DEC; e->a = pole(dec, sr); }
+    if (e->mode == S1U_E_ATK && (S1_ENVELOPE_EXACT ? (double)e->y > .99 : e->y > .99f)) { e->mode = S1U_E_DEC; e->a = pole(dec, sr); }
     return e->y;
 }
 
```

### donor-source.sha256

```text
af1cc2af3799df52e48b57b1cab6c90fffb96134fd4e1f687c5a3a55873476d9  render.c
4c704b15799747314e9b3cff7e50cb8a95b6dd344eeaf685b2cbceeaeae7b4d2  part.c
6cdd06c8f691d62f4fa57e306e4b3ed34db1e0da12e5caed7ec4833d67e24c64  dsp/s1c/s1c.c
cf4f8a9a28a13a0683669f7194608c84db80ad8fe19a512b4e77b8dfeedd44c4  dsp/s1c/s1c.h
```

### source-pins.txt

```text
7fa9df89ede499229f67d267cbfd58914a6a5b3deb41725db96b14d1cab7356e  /workspace/scratch/3a7051e761a0/recovered-sound/s1-room-parity/build-lfo/dsp/s1c/s1u.c
d603d1bbd44d28506bf8fbcd476f4af92e56da060ecc7ac2be90f4e40c58aa80  /workspace/scratch/3a7051e761a0/recovered-sound/s1-room-parity/build-releasephase/dsp/s1c/s1u.c
```

### correct-25-all-summary.json

```json
{
  "count": 906,
  "valid": 906,
  "missing": [],
  "bad_original_hash": [],
  "note_count_mismatch": [],
  "bank_trial_sha_equal": 300,
  "worst_whole": [
    -47.564052112590424,
    193,
    1
  ],
  "worst_nonzero_original_tail": [
    0.0,
    837,
    1,
    1.5308628641993312e-45
  ],
  "worst_absolute_tail_error_dbfs": [
    -68.43266641570763,
    386,
    1
  ],
  "worst_spectrum": [
    1.1670150573799325,
    379,
    0
  ],
  "whole_flags": [],
  "spectral_flags": []
}
```

### exact-history-host.patch

```diff
--- a/exact-delay.c
+++ b/exact-delay.c
@@ -3,19 +3,130 @@
 #include "soundpipe.h"
 #include <math.h>
 #include <stdlib.h>
+#include <stdint.h>
+#include <string.h>
+#include <stdio.h>
 struct dnbox_s1_exact_delay {
     sp_data *sp;
+    unsigned bits,narrow,own[4],at[4],seen[4];
     sp_vdelay *line[4];
     sp_moogladder *lp[2];
     sp_crossfade *mix[2];
 };
+static uint32_t word(float x){uint32_t u;memcpy(&u,&x,4);return u;}
+static float value(uint32_t u){float x;memcpy(&x,&u,4);return x;}
+static void geometry(unsigned b,unsigned *m,unsigned *e,unsigned *bias,unsigned *mask){
+    *e=b>=24?8:b>=18?7:6;*m=b-1-*e;*bias=b>=24?127:b>=18?108:52;*mask=(1u<<*e)-1u;
+}
+static void put(unsigned char *p,unsigned i,unsigned b,float x){
+    unsigned m,e,bs,mask;geometry(b,&m,&e,&bs,&mask);
+    uint32_t u=word(x),sign=(u>>31)<<(b-1),w=u&0x7fffffff;
+    unsigned shift=23-m;
+    w=(w+(((1u<<shift)-1u)>>1)+((w>>shift)&1u))>>shift;
+    int ex=(int)(w>>m)-127+(int)bs;
+    uint32_t code=ex<=0?0u:ex>=(int)mask?((mask<<m)-1u):((uint32_t)ex<<m)|(w&((1u<<m)-1u));
+    code|=sign;
+    size_t b0=(size_t)i*b;unsigned k=(unsigned)b0&7u;unsigned char *at=p+(b0>>3);
+    uint32_t z=0;memcpy(&z,at,(b+k+7u)>>3);
+    z=(z&~(((1u<<b)-1u)<<k))|(code<<k);memcpy(at,&z,(b+k+7u)>>3);
+}
+static float get(const unsigned char *p,unsigned i,unsigned b){
+    unsigned m,e,bs,mask;geometry(b,&m,&e,&bs,&mask);
+    size_t pos=(size_t)i*b; unsigned k=(unsigned)pos&7u;
+    uint32_t z=0;memcpy(&z,p+(pos>>3),(b+k+7u)>>3);
+    z=(z>>k)&((1u<<b)-1u);
+    uint32_t sign=((z>>(b-1))&1u)<<31,ex=(z>>m)&mask;
+    return value(!ex?sign:sign|((127u+ex-bs)<<23)|((z&((1u<<m)-1u))<<(23-m)));
+}
+static float by_age(const unsigned char *buf,int r,int logical,int L,int P,
+                    int cursor,unsigned seen,unsigned bits){
+    int age=L-r;if(age<0)age+=logical;
+    if(age>=P){if((unsigned)age>seen)return 0.f;abort();}
+    int i=cursor-age;if(i<0)i+=P;
+    return get(buf,(unsigned)i,bits);
+}
+static int bounded_compute(sp_data *sp, sp_vdelay *p, unsigned bits,int physical,
+       unsigned *at,unsigned *seen, SPFLOAT *in, SPFLOAT *out)
+{
+    int32_t maxd, indx;
+    *out = p->sr;
+    SPFLOAT del = p->del;
+    SPFLOAT b0, b1, b2, b3;
+    int32_t v0, v1, v2, v3;
+    SPFLOAT fv1;
+    indx = p->left;
+    unsigned char *buf=(unsigned char *)p->buf.ptr;
+
+    put(buf,*at,bits,*in + p->prev*p->feedback);
+
+    fv1 = del * (-1.0 * sp->sr);
+    v1 = (int32_t)fv1;
+    fv1 -= (SPFLOAT) v1;
+    v1 += (int32_t)indx;
+    maxd = (uint32_t) (p->maxdel * p->sr);
+    /* Make sure we're inside the buffer */
+    if ((v1 < 0) || (fv1 < 0.0)) {
+        fv1++; v1--;
+        while (v1 < 0) {
+            v1 += (int32_t)maxd;
+        }
+    } else {
+        while (v1 >= (int32_t)maxd) {
+        v1 -= (int32_t)maxd;
+        }
+    }
+    /* Find next sample for interpolation      */
+    v2 = (v1 == (int32_t)(maxd - 1UL) ? 0L : v1 + 1L);
+
+    if (maxd<4) {
+        b1 = by_age(buf,v1,maxd,indx,physical,(int)*at,*seen,bits);
+        b2 = by_age(buf,v2,maxd,indx,physical,(int)*at,*seen,bits);
+        *out = b1 + fv1 * (b2 - b1);
+    } else {
+        v0 = (v1==0 ? maxd-1 : v1-1);
+        v3 = (v2==(int32_t)maxd-1 ? 0 : v2+1);
+        {
+            SPFLOAT w, x, y, z;
+            z = fv1 * fv1; z--;
+            z *= 0.1666666667;
+            y = fv1;
+            y++; w = (y *= 0.5); w--;
+            x = 3.0 * z; y -= x; w -= z; x -= fv1;
+            b0 = by_age(buf,v0,maxd,indx,physical,(int)*at,*seen,bits);
+            b1 = by_age(buf,v1,maxd,indx,physical,(int)*at,*seen,bits);
+            b2 = by_age(buf,v2,maxd,indx,physical,(int)*at,*seen,bits);
+            b3 = by_age(buf,v3,maxd,indx,physical,(int)*at,*seen,bits);
+            *out = (w*b0 + x*b1 + y*b2 + z*b3)
+            * fv1 + b1;
+        }
+    }
+    if (++indx == maxd) indx = 0;
+    p->left = indx;
+    if (++*at==(unsigned)physical)*at=0;
+    if(*seen<(unsigned)maxd)++*seen;
+    p->prev = *out;
+    return SP_OK;
+}
+
 dnbox_s1_exact_delay *dnbox_s1_delay_new(unsigned rate) {
     dnbox_s1_exact_delay *p=calloc(1,sizeof *p);
     if(!p)return 0;
+    const char *v=getenv("S1_DELAY_WORD_BITS"); p->bits=v?(unsigned)atoi(v):32;
+    if(p->bits!=8 && p->bits!=12 && p->bits!=16 && p->bits!=18 && p->bits!=20 && p->bits!=21 && p->bits!=22 && p->bits!=23 && p->bits!=24 && p->bits!=25 && p->bits!=26 && p->bits!=32)abort();
+    p->narrow=getenv("S1_NARROW_DELAY_HORIZON")!=0;
     sp_create(&p->sp); p->sp->sr=rate; p->sp->nchan=2;
+    size_t payload=0;
     for(unsigned i=0;i<4;i++){
         sp_vdelay_create(&p->line[i]); sp_vdelay_init(p->sp,p->line[i],10.f);
+        if(p->bits!=32){
+          sp_vdelay *l=p->line[i];
+          p->own[i]=p->narrow ? (i<2?5u*rate:5u*rate/2u)+8u : (unsigned)(l->maxdel*rate);
+          const size_t n=((size_t)p->own[i]*p->bits+7u)/8u;
+          payload+=n;
+          sp_auxdata_free(&l->buf);sp_auxdata_alloc(&l->buf,n);
+        }
     }
+    if(p->bits!=32)fprintf(stderr,"exact-source four-line history encoding %u physical bytes %zu at %u Hz, narrow %u\n",p->bits,payload,rate,p->narrow);
     for(unsigned i=0;i<2;i++){
         sp_moogladder_create(&p->lp[i]); sp_moogladder_init(p->sp,p->lp[i]);
         sp_crossfade_create(&p->mix[i]); sp_crossfade_init(p->sp,p->mix[i]);
@@ -29,15 +140,17 @@
     sp_destroy(&p->sp); free(p);
 }
 void dnbox_s1_delay_reset(dnbox_s1_exact_delay *p) {
-    for(unsigned i=0;i<4;i++)sp_vdelay_reset(p->sp,p->line[i]);
+    for(unsigned i=0;i<4;i++){
+     if(p->bits==32)sp_vdelay_reset(p->sp,p->line[i]);
+     else {p->at[i]=p->seen[i]=0;p->line[i]->left=0;memset(p->line[i]->buf.ptr,0,p->line[i]->buf.size);}
+    }
 }
 void dnbox_s1_delay_tap(dnbox_s1_exact_delay *p,float t,float fb,float l,float r,float *left,float *right) {
     for(unsigned i=0;i<4;i++){p->line[i]->del=i<2?t*2.f:t;p->line[i]->feedback=fb;}
     float o=0, fill=0;
-    sp_vdelay_compute(p->sp,p->line[0],&l,left);
-    sp_vdelay_compute(p->sp,p->line[1],&r,&o);
-    sp_vdelay_compute(p->sp,p->line[2],&r,&fill);
-    sp_vdelay_compute(p->sp,p->line[3],&o,right);
+#define TICK(i,in,out) do{ if(p->bits==32)sp_vdelay_compute(p->sp,p->line[i],in,out); else bounded_compute(p->sp,p->line[i],p->bits,(int)p->own[i],&p->at[i],&p->seen[i],in,out); }while(0)
+    TICK(0,&l,left);TICK(1,&r,&o);TICK(2,&r,&fill);TICK(3,&o,right);
+#undef TICK
     *right+=fill;
 }
 void dnbox_s1_delay_insert(dnbox_s1_exact_delay *p,float t,float fb,float ft,float cutoff,float ratio,
```

### summarize.py

```python
from pathlib import Path
import json,sys,math
def src(path):return {v['id']:v for v in map(json.loads,Path(path).read_text().splitlines()[1:])}
a,b=map(src,sys.argv[1:]); asserts={}
valid={i:r for i,r in a.items() if 'metrics' in r}
missing=[i for i in a if i not in valid]
ref_mismatch=[i for i,r in valid.items() if r['ref_sha256']!=b[i]['ref_sha256']]
note_mismatch=[i for i,r in valid.items() if r['same_note_events']!=b[i]['same_note_events']]
assert not ref_mismatch and not note_mismatch
fail={}; whole=[]; tail=[]; bands=[]; abs_err=[]; whole_flags=[]; spectral_flags=[]
for i,r in valid.items():
 m=r['metrics']; q=b[i]['metrics']
 for ch in range(2):
  db=m['whole']['residual_dbr'][ch]; d=m['tail_spectral_rms_db'][ch]
  total=m['last_second']['reference_rms'][ch]; te=m['last_second']['residual_dbr'][ch]
  whole.append((db,i,ch)); bands.append((d,i,ch))
  if total>0:
   tail.append((te,i,ch,total))
   abs_err.append((20*math.log10(total)+te,i,ch))
  if db>-40:whole_flags.append((i,ch,db,q['whole']['residual_dbr'][ch]))
  if d>=3:spectral_flags.append((i,ch,d,q['tail_spectral_rms_db'][ch],total))
print(json.dumps(dict(count=len(a),valid=len(valid),missing=missing,bad_original_hash=ref_mismatch,
 note_count_mismatch=note_mismatch,bank_trial_sha_equal=sum(r['trial_sha256']==b[i]['trial_sha256'] for i,r in valid.items()),
 worst_whole=max(whole,default=None),worst_nonzero_original_tail=max(tail,default=None),
 worst_absolute_tail_error_dbfs=max(abs_err,default=None),worst_spectrum=max(bands,default=None),
 whole_flags=whole_flags,spectral_flags=spectral_flags),indent=2))
```
