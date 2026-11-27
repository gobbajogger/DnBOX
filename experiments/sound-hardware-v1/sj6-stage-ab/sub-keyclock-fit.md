# Fit the guarded Sub clock inlet without shifting every engine field

The original per-key Sub tuning was proven separately in [sub-keyclock.md](sub-keyclock.md). The full recovered warm/Fold32 experimental route with typed inlet overflowed XIP by 924 B Elite / 916 B Pro, unbound; it is not suitable merely to enable a pitch input. A minimal standalone per-part pointer preserved signed host guards but moved embedded juno/engine field offsets, changing large numbers of LTO functions. On the joined+Delay-clock donor this smaller per-part version still overflowed by 224/212 B. Neither candidate was adopted.

The same original instrument tuning table is immutable and not a per-preset effect. A common pointer can be installed by explicitly validated startup code before the first typed SuperJ6 owner is initialized. Both owners consume the same canonical immutable profile, use their independent note, physical range and voice state, and cannot rebind after initialization. Legacy engine and no-profile fallbacks are unchanged. The structural API is not authority or external format provenance. The actual map and sample payload are not included; the copied source change is a diagnostic inlet only. There is no registered production caller, no physical board write, no claim that the fixed drift is active.

Host guards: no-profile and neutral zero-profile arms match feature off for 192 irregular signed stereo blocks (digest e6bde0debf7e8927). C ABI size unchanged for all three arms (2604 host bytes). Synthetic nonzero profile checks distinct simultaneous typed physical ranges, correct per-key phase owners, and one-time install rejection on invalid, duplicate or late bind. Floating point output untouched beyond chosen Sub note-step/range. Only authenticated low-range path was retained; unsupported high/out-of-range behavior not invented.

Paired PI32 build of the ordinary joined+Delay-clock donor with no caller, but a volatile fetch keeping the renderer branch live: **396/408 B free XIP** and **3236 B** usable pool. Cost 128 B XIP versus identical clock donor. Startup binder and table storage were not retained by the LTO full link because no production caller is present. Do not claim they fit on those figures. Independent of short-rate refinement.

Reproduce from the joined+clock donor; apply the patch, enable X0X_SJ6_SUB_TUNE_PROFILE for the diagnostic builds. Compare the test executable with no args, built with flag 0 and 1, against the flag 1 test with argument 1 (zero profile). Then run the flag 1 test with argument 2 (synthetic profiles). Compile using -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DX0X_SJ6_SUB_TUNE_PROFILE=K -O2 -ffp-contract=off -ffunction-sections -fdata-sections, include firmware/src/dsp and firmware/src, and link with -Wl,--gc-sections -lm.


### shared-inlet.patch

```diff
--- a/firmware/src/dsp/dnbx_core.c
+++ b/firmware/src/dsp/dnbx_core.c
@@ -384,6 +384,10 @@
     return e->level;
 }
 
+#if X0X_SJ6_NOISE_SUB_NATIVE && X0X_SJ6_SUB_TUNE_PROFILE
+static const dnbx_sj6_sub_tune_profile *volatile sj6_profile;
+static uint8_t sj6_profile_closed;
+#endif
 uint32_t dnbx_abi_version(void) { return DNBX_ABI_VERSION; }
 uint32_t dnbx_diagnostic_counters_enabled(void) { return DNBX_DIAGNOSTIC_COUNTERS; }
 size_t dnbx_part_size(void) { return sizeof(dnbx_part); }
@@ -409,7 +413,12 @@
 int dnbx_init_superj6(dnbx_part *s, float sr)
 {
     int status = dnbx_init(s, sr);
-    if (status == DNBX_OK) s->superj6_amp_range = 1;
+    if (status == DNBX_OK) {
+        s->superj6_amp_range = 1;
+#if X0X_SJ6_NOISE_SUB_NATIVE && X0X_SJ6_SUB_TUNE_PROFILE
+        sj6_profile_closed=1;
+#endif
+    }
     return status;
 }
 static float filter_sustain(const dnbx_part *s,unsigned l)
@@ -854,6 +863,24 @@
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
+int dnbx_sj6_install_sub_tune_profile(const dnbx_sj6_sub_tune_profile *p,size_t n)
+{
+    if(sj6_profile_closed||sj6_profile||!p)return 0;
+    if(n!=sizeof(*p)||p->version!=1||p->units!=1||p->reserved[0]||p->reserved[1])return 0;
+    for(unsigned k=0;k<128;k++){
+        int32_t v=sj6_sub_tune_mc(p,k);
+        if(v< -100000 || v>100000)return 0;
+    }
+    sj6_profile=p;return 1;
+}
+#endif
 static unsigned sj6_sub_zone(unsigned note)
 { return note<34u ? 0u : note>=94u ? 11u : (note-34u)/6u+1u; }
 static inline float sj6_synthesize_sub(float phase,float step,const uint8_t *h)
@@ -891,12 +918,26 @@
     float sj6_steps[DNBX_VOICES];
     const uint8_t *sj6_harmonics[DNBX_VOICES];
     uint32_t sj6_noise=s->sj6_noise_random;
-    if(s->sj6_sub_gain)
+    if(s->sj6_sub_gain) {
+#if X0X_SJ6_SUB_TUNE_PROFILE
+        const dnbx_sj6_sub_tune_profile *profile=s->superj6_amp_range?sj6_profile:0;
+        float range;
+        const int shift=profile && dnbx_read_raw(s,86u,&range)==DNBX_OK && range==0.f ? -12:0;
+#endif
         for(unsigned i=0;i<DNBX_VOICES;++i) {
             unsigned note=s->voice[i].note;
-            sj6_steps[i]=.5f*pitch_increment(s,note,0.f)*source_bend;
+            float offset=0.f;
+#if X0X_SJ6_SUB_TUNE_PROFILE
+            if(profile){
+                const unsigned ranged=(unsigned)((int)note+shift);
+                if(ranged<=127u)note=ranged;
+                offset=(float)sj6_sub_tune_mc(profile,note)*.00001f;
+            }
+#endif
+            sj6_steps[i]=.5f*pitch_increment(s,note,offset)*source_bend;
             sj6_harmonics[i]=sj6_sub_harmonic[sj6_sub_zone(note)];
         }
+    }
 #else
     (void)source_bend;
 #endif
--- a/firmware/src/dsp/dnbx_core.h
+++ b/firmware/src/dsp/dnbx_core.h
@@ -207,6 +207,17 @@
 #endif
 #if X0X_SJ6_NOISE_SUB_NATIVE
 void dnbx_set_sj6_sources(dnbx_part *,unsigned owner,const uint8_t code[2]);
+#if X0X_SJ6_SUB_TUNE_PROFILE
+/* Caller owns already-authorized, immutable RAM or memory-mapped storage
+ * before any typed owner starts. This leaf validates format, not provenance.
+ * Packed signed 24-bit little-endian milli-cents indexed by physical key.
+ * It is shared read-only across typed owners and cannot be rebound. */
+typedef struct dnbx_sj6_sub_tune_profile {
+    uint8_t version, units, reserved[2], milli_cents[128][3];
+} dnbx_sj6_sub_tune_profile;
+int dnbx_sj6_install_sub_tune_profile(const dnbx_sj6_sub_tune_profile *,size_t);
+#endif
+
 #endif
 #if X0X_SJ6_VINTAGE_NATIVE
 float dnbx_sj6_vintage_cents(unsigned position);
```

### test.c

```c
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "dnbx_core.c"
static uint64_t sig=1469598103934665603ull;
static void hash(const float*x,size_t n) {
 const uint8_t *b=(const uint8_t*)x;
 for(size_t j=0;j<n*sizeof(float);++j)sig=(sig^b[j])*1099511628211ull;
}
static void init(dnbx_part*p,unsigned sub,float range) {
 assert(dnbx_init_superj6(p,44100.f)==DNBX_OK);
 uint8_t v[2]={0,sub};dnbx_set_sj6_sources(p,0,v);
 assert(dnbx_store_raw(p,86,range)==DNBX_RAW_STORED_ONLY);
}
static void render(dnbx_part*p,float*l,float*r,unsigned n) {
 assert(dnbx_render_pitch_osc_levels_velocity(p,l,r,n,1.0125f,.5f,.3f,0.f)==DNBX_OK);
}
#if X0X_SJ6_SUB_TUNE_PROFILE
static dnbx_sj6_sub_tune_profile zero={1,1,{0,0},{{0}}},low={1,1,{0,0},{{0}}},high={1,1,{0,0},{{0}}};
static void put(dnbx_sj6_sub_tune_profile*p,unsigned note,int32_t v) {
 for(unsigned j=0;j<3;j++)p->milli_cents[note][j]=(uint8_t)((uint32_t)v>>(8*j));
}
static float pred(dnbx_part*s,unsigned note,int32_t mc){
 return .5f*pitch_increment(s,note,(float)mc*.00001f)*1.0125f;
}
static void check(dnbx_part*p,unsigned incoming,unsigned physical,int32_t t){
 const int slot=dnbx_note_on(p,incoming,86);
 assert(slot>=0&&slot<6);
 const float before=p->voice[slot].sj6_sub_phase;
 float l,r;render(p,&l,&r,1);
 float updated=before+pred(p,physical,t); if(updated>=1.f)updated-=1.f;
 assert(p->voice[slot].sj6_sub_phase==updated);
}
#endif
int main(int argc,char **argv){
 dnbx_part x,y;float l[129],r[129],a[129],b[129];
 #if X0X_SJ6_SUB_TUNE_PROFILE
 int mode=argc>1?argv[1][0]-'0':0;
 if(mode){
   dnbx_sj6_sub_tune_profile bad={1,1,{1,0},{{0}}};
   assert(!dnbx_sj6_install_sub_tune_profile(&bad,sizeof bad));
   put(&low,36,-90000);put(&low,55,35000);
   assert(!dnbx_sj6_install_sub_tune_profile(&low,sizeof low-1));
   assert(dnbx_sj6_install_sub_tune_profile(mode==2?&low:&zero,sizeof zero));
   assert(!dnbx_sj6_install_sub_tune_profile(&high,sizeof high));
 }
 #endif
 init(&x,115,.5f);init(&y,115,.5f);
 #if X0X_SJ6_SUB_TUNE_PROFILE
 assert(!dnbx_sj6_install_sub_tune_profile(&zero,sizeof zero));
 if(mode==2){
    init(&x,115,0.f);
    check(&x,48,36,-90000);
    check(&y,55,55,35000);
    assert(dnbx_all_off(&x)==DNBX_OK);
    assert(!dnbx_sj6_install_sub_tune_profile(0,0));
    return 0;
 }
 #endif
 for(unsigned block=0;block<192;block++){
  unsigned note=(block*37)%128, n=(block*29)%128+1;
  if(block%4<=1)assert(dnbx_note_on(&x,note,86)==dnbx_note_on(&y,note,86));
  else assert(dnbx_note_off(&x,note)==dnbx_note_off(&y,note));
  render(&x,l,r,n);render(&y,a,b,n);
  assert(!memcmp(l,a,n*4)&&!memcmp(r,b,n*4));
  hash(l,n);hash(r,n);
 }
 printf("stock_digest=%016llx part_bytes=%zu\n",(unsigned long long)sig,sizeof x);
}
```

```text
guarded elite XIP 552276 free 396 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000001000000d8ef00007def59ef2529a63b6ea959b91b5e557d ELF ae6f864ea1f1c044b48c8bd0970dcb5c3f4ddf82c8aa9a82373ff9c7277b0c36
guarded pro XIP 552264 free 408 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000002000000d8ef00007def59ef65d268ccaaad0e847dd15664 ELF 473f462945ae1331863ee7956cc5c35c91f1dbb63352be22d651d181a42bb3f6
```
