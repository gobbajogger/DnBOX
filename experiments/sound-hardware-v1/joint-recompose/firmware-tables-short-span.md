# Independent firmware optimization: exact tables and real short-span attribution

## Shared constants, verified on both target links

Move the identical 64 B IMA rounding table into the same definition unit as the existing shared step/index tables (BREAK and 909). Use one actual native 52 B beat grid for MIX and Chicago Delay, retaining independent original expression order, and identical generated constants for independent host-only renderers. The source does not change effects behavior, feature access or UI.

Baseline-to-patched Elite/Pro free XIP: **540/552 → 656/668 B**, exactly **116 B saved** on both actual PI32 links and unchanged usable audio pool **3,236 B**. Both target symbol tables show one 64 B correction constant and one 52 B beat grid. GCC optimized and ASan/UBSan IMA comparison: 3,193,789 exact per arm, and BREAK original plus click negative/positive cases passed. The old independent Chicago test expects bass303.h (missing in reconstructed donor); a narrower compiled real-header replacement tested all 256 ordinals×381 BPMs against original, with raw byte equality for both operation orders, standalone and shared-grid compilation. Host Clang binary not present; GCC sanitizer used with leak detection off due to ptrace environment. No release/active audio deadline claim.

## Setup overhead: ruled out as the dominant dropout source in the measured cue

Independent experimental witness, **not** combined with the table patch. Paired native strict replay with the exact joined baseline plus 48 B setup counters. Both paths actually audible (458/454 spans) and all five typed readbacks. At most 32 frames: 17 spans and 286 frames on both (about 0.29% of coaudible frames). Longer: 441/437 spans and 99,298/98,274 frames. Sum of two owners' setup plus subsequent Core0 setup was only ~19.82 ms / 19.65 ms over ~3.09/3.08 s of coaudible render wall, an aggregate ~0.64% conservative ratio; some owner times overlap. Short spans accounted for ~0.60% of the aggregate render wall time. This does not prove a peak-bound for all patches or word events, but the tested loaded dropout workload will not be fixed by removing these setup calls. Measured full peak 26,343/26,622 µs and 167/165 XRUNs with instrumentation, no native pass or A/B speed claim. Leave its recorder off in actual candidates.


### dedup.patch

```diff
--- a/firmware/src/dsp/ima_adpcm.h
+++ b/firmware/src/dsp/ima_adpcm.h
@@ -20,18 +20,19 @@
     4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
     22385, 24623, 27086, 29794, 32767};
 const int8_t IMA_IDX_T[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
-#else
-extern const int16_t IMA_STEP_T[89];
-extern const int8_t IMA_IDX_T[8];
-#endif
-
-/* decode nibble i of d; updates the decoder state and returns the new sample */
-static const uint8_t IMA_ROUND_CORR[64] = {
+const uint8_t IMA_ROUND_CORR[64] = {
     0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0, 0,1,0,1,0,1,0,1,
     0,0,0,0,0,0,0,0, 0,0,1,1,0,0,1,1,
     0,1,0,1,0,1,0,1, 0,1,1,2,0,1,1,2
 };
+#else
+extern const int16_t IMA_STEP_T[89];
+extern const int8_t IMA_IDX_T[8];
+extern const uint8_t IMA_ROUND_CORR[64];
+#endif
+
+/* decode nibble i of d; updates the decoder state and returns the new sample */
 static inline int32_t ima_decode_code(uint32_t code, int32_t *pred, int32_t *idx)
 {
     uint32_t mag = code & 7u;
--- a/firmware/src/dsp/fxbus.c
+++ b/firmware/src/dsp/fxbus.c
@@ -2,6 +2,7 @@
 /* Send reverb, synced delay and master stage: a port of 9W9's er99_verb_tick,
  * er99_dly_tick, master distortion and master_glue (GPL-3.0). See fxbus.h. */
 #include "fxbus.h"
+#include "fx_beat_table.h"
 #include "../x0x_cold.h"          /* X0X_COLD: fxbus_init is boot only, not the send/render */
 
 static const int32_t fx_cl[4] = { 1116, 1188, 1277, 1356 };
@@ -10,10 +11,7 @@
 #define FX_RV_QUIET (1356 + 8192)          /* combs silent this long: the allpass tail is < -140 dB */
 
 /* 1/32 1/16T 1/16 1/8T 1/16. 1/8 1/4T 1/8. 1/4 1/2T 1/4. 1/2 1/2. (in beats) */
-static const float fx_beats[13] = {
-    0.125f, 1.0f / 6.0f, 0.25f, 1.0f / 3.0f, 0.375f, 0.5f, 2.0f / 3.0f,
-    0.75f, 1.0f, 4.0f / 3.0f, 1.5f, 2.0f, 3.0f
-};
+const float x0x_fx_beats[13] = X0X_FX_BEATS_VALUES;
 
 /* ---- parameters ---- */
 enum { CV_LIN, CV_EXP, CV_SW };
@@ -89,7 +87,7 @@
     if (i < 0) i = 0;
     if (i > 12) i = 12;
     const float bpm = f->bpm >= 20.0f ? f->bpm : 120.0f;
-    float ms = fx_beats[i] * 60000.0f / bpm;
+    float ms = x0x_fx_beats[i] * 60000.0f / bpm;
     const float max_ms = (float)(f->dlen - 256) / D9_SR * 1000.0f;
     while (ms > max_ms)                 /* longer than the line (1/2. under 90 BPM): half of it, */
         ms *= 0.5f;                     /* which is still on the beat; cutting it short is not */
--- a/firmware/src/app/chicago_bpmdly.h
+++ b/firmware/src/app/chicago_bpmdly.h
@@ -43,15 +43,17 @@
 }
 
 /* Same beat grid as fxbus DlTime 0..12. */
+#include "../dsp/fx_beat_table.h"
 static inline float chi_bpmdly_beats(uint8_t div)
 {
-    static const float beats[13] = {
-        0.125f, 1.0f / 6.0f, 0.25f, 1.0f / 3.0f, 0.375f, 0.5f, 2.0f / 3.0f,
-        0.75f,  1.0f,        4.0f / 3.0f, 1.5f,  2.0f,  3.0f
-    };
     if (div > 12)
         div = 12;
+#ifdef __PI32V2__
+    return x0x_fx_beats[div];
+#else
+    static const float beats[13] = X0X_FX_BEATS_VALUES;
     return beats[div];
+#endif
 }
 
 static inline void chi_bpmdly_set_bpm(chi_bpmdly_t *d, uint8_t e1, uint8_t e2, float sr, float bpm)
--- /dev/null
+++ b/firmware/src/dsp/fx_beat_table.h
@@ -0,0 +1,5 @@
+/* SPDX-License-Identifier: GPL-3.0-only */
+#pragma once
+/* Shared definition for Mix Delay and Chicago Delay. */
+#define X0X_FX_BEATS_VALUES {    0.125f, 1.0f / 6.0f, 0.25f, 1.0f / 3.0f, 0.375f, 0.5f, 2.0f / 3.0f,    0.75f, 1.0f, 4.0f / 3.0f, 1.5f, 2.0f, 3.0f}
+extern const float x0x_fx_beats[13];
```

### beat-check.c

```c
#include <stdio.h>
#include <stdint.h>
#include "chicago_bpmdly.h"
#ifdef __PI32V2__
const float x0x_fx_beats[13]=X0X_FX_BEATS_VALUES;
#endif
int main(void){
 for(unsigned v=0;v<256;v++){
  float x=chi_bpmdly_beats(v);
  for(unsigned b=20;b<=400;b++){
   float tempo=(float)b;
   float hz[2]={x*(60000.f/tempo),(x*60000.f)/tempo};
   if(fwrite(hz,sizeof hz,1,stdout)!=1)return 1;
  }
 }
 return 0;
}
```

### recorder.patch

```diff
--- a/firmware/src/app/engine.c
+++ b/firmware/src/app/engine.c
@@ -2508,6 +2508,16 @@
 
 #include "original_presets.inc"
 
+/* Instrumented-only causal setup witness. Core A and B publish to separate
+ * slots, consumed only after the render_sub join. The between-span call is
+ * later and Core0-exclusive. No event boundary or prepare elision. */
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+#if !DNBOX_LAB_JOINT_FIXED_RENDER_CUE
+#error "short-span setup witness is lab-only"
+#endif
+volatile uint32_t lab_short_setup[2][5]; /* cnt, frames, A+B pre ticks, post ticks, render wall ticks */
+static volatile uint32_t lab_short_owner_ticks[2];
+#endif
 static void original_prepare_all(void)
 {
     float bpm=seq_tempo(&seq);
@@ -3231,7 +3241,13 @@
         float *bb[NBASS];
         bb[bv] = bl; bb[1 - bv] = br;                     /* bass B renders right-left, as always */
         PROF_T(pb);
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+        uint32_t lab_setup_stamp=plat_ticks24();
+#endif
         (void)original_runtime_prepare(&original_bass[bv], seq_tempo(&seq));
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+        lab_short_owner_ticks[bv]=plat_ticks24()-lab_setup_stamp;
+#endif
         bass_adapt_pre(bv, (unsigned)n);
         int status = original_runtime_render(&original_bass[bv], bv ? &original_workspace_b : &original_workspace,
                                              bb[bv], bb[1-bv], n);
@@ -3715,13 +3731,36 @@
         /* any stretch of ENG_TAIL_CS or more: pipeline. Mastered frames
          * must be a prefix (the master's state is in sample order). */
         eng_do_master = (k >= ENG_TAIL_CS && done == eng_mastered);
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+        const unsigned lab_active=lab_joint_fixed_cue_seeded==1u;
+        lab_short_owner_ticks[0]=lab_short_owner_ticks[1]=0;
+        const uint32_t lab_render_from=plat_ticks24();
+#endif
         render_sub(out_l + done, out_r + done, k, 0);
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+        const uint32_t lab_render_elapsed=plat_ticks24()-lab_render_from;
+        const unsigned lab_coaudible=lab_active && bass_render_ok[0] && bass_render_ok[1] &&
+            bass_quiet_next[0]<k && bass_quiet_next[1]<k &&
+            original_bass[0].kind==ORIGINAL_SOUND_SYNTHONE &&
+            original_bass[1].kind==ORIGINAL_SOUND_JUNO;
+#endif
         d8_frame+=k;
         if (eng_do_master && eng_master_done)
             eng_mastered = done + k;
         {
             PROF_T(pt);
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+            const uint32_t lab_post_from=plat_ticks24();
+#endif
             original_prepare_all(); /* next actual sequencer stretch, before its events */
+#if DNBOX_LAB_SHORT_SPAN_SETUP
+            const uint32_t lab_post_elapsed=plat_ticks24()-lab_post_from;
+            if(lab_coaudible) {
+                volatile uint32_t *a=lab_short_setup[k>32u];
+                a[0]++; a[1]+=k; a[2]+=lab_short_owner_ticks[0]+lab_short_owner_ticks[1];
+                a[3]+=lab_post_elapsed; a[4]+=lab_render_elapsed;
+            }
+#endif
             seq_advance(&seq, k, &SINK);
             brk_midi_drain();
             PROF_ADD(7, pt);
```

### read.py

```python
"""Decode a separately-built, fail-closed lab-only run, no native invocation."""
import json,re,struct,subprocess,sys
from pathlib import Path
nm,elf,ram=map(Path,sys.argv[1:])
sym=re.search(r'^([a-fA-F0-9]+)\s+\w\s+lab_short_setup$',subprocess.check_output([nm,'-n',elf],text=True),re.M)
if sym is None: raise SystemExit('missing instrumented symbol; not a measured zero')
s=ram.read_bytes()
off=int(sym[1],16)-0x1c00000
if len(s)!=1048576 or not 0<=off<=len(s)-40 or off%4: raise SystemExit('wrong RAM geometry')
vals=struct.unpack_from('<10I',s,off)
def bucket(d,small):
    ct,frames,pre,post,render=d
    if (not ct and any(d)) or (ct and not (ct*(1 if small else 33)<=frames<=ct*(32 if small else 256))):
        raise SystemExit('invalid aggregate')
    return dict(spans=ct,frames=frames,owners_total_us=pre/24,post_us=post/24,render_wall_us=render/24)
print(json.dumps(dict(at_most_32=bucket(vals[:5],True),longer=bucket(vals[5:],False),
    all_elapsed_has_nested_interrupts=True,coaudible_joint_scope_only=True)))
```

### short-elite.json

```json
{"at_most_32": {"spans": 17, "frames": 286, "owners_total_us": 92.08333333333333, "post_us": 635.0, "render_wall_us": 18473.125}, "longer": {"spans": 441, "frames": 99298, "owners_total_us": 2599.4583333333335, "post_us": 16495.583333333332, "render_wall_us": 3068798.8333333335}, "all_elapsed_has_nested_interrupts": true, "coaudible_joint_scope_only": true}
```

### short-pro.json

```json
{"at_most_32": {"spans": 17, "frames": 286, "owners_total_us": 130.41666666666666, "post_us": 619.875, "render_wall_us": 18411.791666666668}, "longer": {"spans": 437, "frames": 98274, "owners_total_us": 2571.1666666666665, "post_us": 16331.375, "render_wall_us": 3058385.5833333335}, "all_elapsed_has_nested_interrupts": true, "coaudible_joint_scope_only": true}
```

```text
guarded elite XIP 552016 free 656 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000001000000d8ef00007def59ef6f6937127cd1ca1ad8953c3b ELF 0c5de3b0527c6e6f21dc8cd6326b69c59429bc3968af4813819ec2cd9c4d733d
guarded pro XIP 552004 free 668 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000002000000d8ef00007def59efa85fb5db104739cf59077d73 ELF fc4ea2207c3f5de9f2b9279b54c3f15fe866945e7cc2561678f6910d08ac8b54
```
