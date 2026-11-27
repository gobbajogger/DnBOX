# Two independent firmware-space candidates for the joint diagnostic

## 909 scratch and list-table relocation

The 909 job no longer materializes a 256-float mono scratch block. Sends and meter consume the same unscaled L/R sum before pan; each destination keeps its prior arithmetic order, all independent coefficient snaps and zero-send short circuits, and the original mixing publication. The 808 path is unchanged. Move the 648 B foreground UI `list_rows` object out of the audio pool to the vacated ordinary BSS; all references are internal value reads/writes in `ui.c`, not pointers retained by DMA or an audio owner.

Compiled UBSan differential using actual old/new functions: 1,052,672 input frames and exact output spans, glides and meters. Signed zero, every block length 1–256, settled/moving glides and near-snap controls are covered. Both native links: **+648 B usable pool** and **−224 B free XIP** relative to the original same diagnostic; Elite/Pro XIP free 564/576 B and usable pool after 8 KiB reserve 3,236 B. Earlier headroom already contains the prior BREAK reclaim, do not double-count.

Strict native replay, same typed 2-engine cue, both independently audible and five owner readbacks: Elite 26,090 µs/163 XRUNs; Pro 23,720 µs/192 XRUNs (baseline 25,851/169 and 23,286/198). Footprint gain, mixed timing tradeoff, not a release pass. No physical device writes.

## Exact tempo-grid storage deduplication

Native SuperJ6 ALT3 and Room sync share the same 72-byte Float32 grid. Share the constant without changing either index calculation or host's separate original double precision. Signed C comparison matches 2,097,152 values. Independent guarded paired links save exactly **72 B XIP** each (Elite/Pro 860/872 B free) with unchanged 2,588 B audio pool. Linked pointer-bearing metadata naturally moves, attributed region spans do not. This is not a strict runtime timing claim. Do not sum free-space values across independent images; the deltas need a combined link.

## Replay

Start on the exact c24f2f2f reconstruction. Apply either patch independently; run the existing paired diagnostic linker. For the 909 proof put `verify.py` at firmware-fit-next with the patched tree beneath it and the unchanged recomposed donor at the path shown by the script. `run-native-check.sh` uses the same verified original read-only project seed and strict replay from that directory. For the grid run `cc -O2 -std=c11 -ffp-contract=off tables-proof.c -o tables-proof` then execute it.


### mono-relocate.patch

```diff
--- a/firmware/src/app/engine.c
+++ b/firmware/src/app/engine.c
@@ -2873,7 +2873,7 @@
 
 /* the mix of one stretch between two events: dry left and right, the reverb and delay sends;
  * t0..tr the 808's scratch, kbuf the break's, mono the 909's kit send */
-static float dry_l[256], dry_r[256], rev[256], dly[256], t1[256], t2[256], kbuf[256], mono[256];
+static float dry_l[256], dry_r[256], rev[256], dly[256], t1[256], t2[256], kbuf[256];
 /* The 808's L/R scratch is the stretch's own out_l/out_r window (saves 2 KiB).
  * Lifetime: core 2 writes it while core 1 only touches dry_l..mono; core 1
  * reads it after plat_cpu2_wait and before the 303s overwrite that window;
@@ -2973,6 +2973,28 @@
     add_glide(rev, m, &mix_glide[b][GL_REV], n);
     add_glide(dly, m, &mix_glide[b][GL_DLY], n);
     meter(b, m, mix_level[b], n);
+}
+
+/* JB_909 owns both unscaled sides until eng_stage 1. Fuse the center
+ * read with its kit sends/meter, without storing a separate 256-float span. */
+__attribute__((noinline)) static void kit_sends_909_unscaled(uint32_t n)
+{
+    glide_t *const rv=&mix_glide[PART_909][GL_REV], *const dl=&mix_glide[PART_909][GL_DLY];
+    float rc=rv->cur, dc=dl->cur, rt=rv->tgt, dt=dl->tgt;
+    const int rm=rc!=rt, dm=dc!=dt, ra=rm||rc!=0.f, da=dm||dc!=0.f;
+    float pk=part_peak[PART_909], g=mix_level[PART_909];
+    for(uint32_t i=0;i<n;i++){
+        const float m=(dry_l[i]+dry_r[i])*0.5f;
+        if(rm)rc+=(rt-rc)*GLIDE_A;
+        if(dm)dc+=(dt-dc)*GLIDE_A;
+        if(ra)rev[i]+=m*rc;
+        if(da)dly[i]+=m*dc;
+        const float a=fm_fabsf(m)*g;
+        if(a>pk)pk=a;
+    }
+    if(rm)rv->cur=fm_fabsf(rt-rc)<1e-5f?rt:rc;
+    if(dm)dl->cur=fm_fabsf(dt-dc)<1e-5f?dt:dc;
+    part_peak[PART_909]=pk;
 }
 
 /* ---- the two cores' jobs, balanced per stretch ----
@@ -3155,13 +3177,11 @@
             drum909_render_st_cleared(&d909, dry_l, dry_r, rev, dly, (int)n, smc_audible_drums(solo, PART_909));
             mix_targets(PART_909, mix_level[PART_909], solo_parts);
             bfx_shape(PART_909, dry_l, dry_r, rev, dly, 4, (int)n);
-            for (i = 0; i < n; i++)                       /* kit send: filtered sides, still centred */
-                mono[i] = (dry_l[i] + dry_r[i]) * 0.5f;
-            scale_glide(dry_l, &mix_glide[PART_909][GL_L], n);   /* then the part's PAN over the kit */
-            scale_glide(dry_r, &mix_glide[PART_909][GL_R], n);
             scale_glide(rev, &mix_glide[PART_909][GL_VREV], n);
             scale_glide(dly, &mix_glide[PART_909][GL_VDLY], n);
-            kit_sends(PART_909, mono, n);
+            kit_sends_909_unscaled(n);
+            scale_glide(dry_l, &mix_glide[PART_909][GL_L], n);
+            scale_glide(dry_r, &mix_glide[PART_909][GL_R], n);
         }
         if (!core)
             PROF_ADD(PR_909, p9);
--- a/firmware/src/app/ui.c
+++ b/firmware/src/app/ui.c
@@ -3785,7 +3785,7 @@
 #else
 #define LIST_MAX 64
 #endif
-static pref_t list_rows[LIST_MAX] X0X_POOL;
+static pref_t list_rows[LIST_MAX];
 static char list_title[16];
 
 static void list_add(pref_t r)
```

### verify.py

```python
from pathlib import Path
import subprocess,re
p=Path(__file__).resolve().parent
ref=p.parent/'s1-organ-wet-recovery/native-ed-brk-recompose/tree/firmware/src/app/engine.c'
cand=p/'tree/firmware/src/app/engine.c'
def funcs(file,names):
 s=file.read_text()
 out=[]
 for n in names:
  a=re.search(r'(?m)^.*static void '+n+r'\(',s).start()
  b=s.index('{',a)
  depth=1
  while depth:
   b+=1; depth+=(s[b]=='{')-(s[b]=='}')
  out.append(s[a:b+1])
 return '\n'.join(out)
a=funcs(ref,['meter','add_glide','scale_glide','kit_sends'])
b=funcs(cand,['kit_sends_909_unscaled'])
preamble=r'''
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#define fm_fabsf fabsf
#define GLIDE_A 0.0022651f
#define PART_909 0
enum { GL_L, GL_R, GL_REV, GL_DLY, GL_VREV, GL_VDLY, GL_N };
typedef struct { float cur,tgt; } glide_t;
typedef struct {float s[4][256];glide_t gl[GL_N];float peak,level;} state;
static state q;
#define dry_l q.s[0]
#define dry_r q.s[1]
#define rev q.s[2]
#define dly q.s[3]
static float part_peak[1],mix_level[1];
static glide_t mix_glide[1][GL_N];
'''
check=r'''
static state process(state x,unsigned k,unsigned n) {
 q=x;memcpy(mix_glide[0],q.gl,sizeof q.gl);
 part_peak[0]=q.peak;mix_level[0]=q.level;
 if(k==0){
  float m[256];
  for(unsigned i=0;i<n;i++)m[i]=(dry_l[i]+dry_r[i])*0.5f;
  scale_glide(dry_l,&mix_glide[0][GL_L],n);
  scale_glide(dry_r,&mix_glide[0][GL_R],n);
  scale_glide(rev,&mix_glide[0][GL_VREV],n);
  scale_glide(dly,&mix_glide[0][GL_VDLY],n);
  kit_sends(0,m,n);
 }else{
  scale_glide(rev,&mix_glide[0][GL_VREV],n);
  scale_glide(dly,&mix_glide[0][GL_VDLY],n);
  kit_sends_909_unscaled(n);
  scale_glide(dry_l,&mix_glide[0][GL_L],n);
  scale_glide(dry_r,&mix_glide[0][GL_R],n);
 }
 memcpy(q.gl,mix_glide[0],sizeof q.gl);q.peak=part_peak[0];return q;
}
static uint32_t r=1;
static uint32_t step(void){return r=r*1664525u+1013904223u;}
static float value(void){return (float)(int16_t)(step()>>16)*0x1p-15f;}
int main(void){
 unsigned long samples=0;
 for(unsigned k=0;k<8192;k++){
  state x={0};x.level=fabsf(value());x.peak=fabsf(value());
  for(unsigned j=0;j<GL_N;j++){
   float a=value(),b=value();
   switch((k>>(j%4))&3){case 0:a=b=0;break;case 1:a=b;break;case 2:b=a+0.000001f;break;}
   x.gl[j]=(glide_t){a,b};
  }
  for(unsigned i=0;i<256;i++)for(unsigned j=0;j<4;j++)
   x.s[j][i]=(i%13==10)?-0.f:(i%11==6)?0.f:value();
  unsigned n=(k&255)+1;
  state a=process(x,0,n),b=process(x,1,n);
  if(memcmp(&a,&b,sizeof a)){fprintf(stderr,"diverged on %u %u\n",k,n);return 1;}
  samples+=n;
 }
 printf("%lu exact frames (all stereo/sends, glides and meters)\n",samples);
}
'''
out=p/'check.c';out.write_text('\n'.join((preamble,a,b,check)))
subprocess.run(['cc','-O2','-std=c11','-ffp-contract=off','-fsanitize=undefined','-fno-sanitize-recover=all',str(out),'-o',str(p/'check')],check=True)
subprocess.run([str(p/'check')],check=True)
```

### run-native-check.sh

```bash
#!/usr/bin/env bash
set -euo pipefail
R=$(cd "$(dirname "$0")/.." && pwd)
S="$R/s1-organ-wet-recovery/native-ed-brk-recompose"
H="$R/restored-harness-inspect/helpers"
O="$R/remote-typed-load"
for d in "$R/firmware-fit-next"; do
  e="$d/tree/joint-diagnostic-later/elite/never-install.elf"
  p="$d/tree/joint-diagnostic-later/pro/never-install.elf"
  (
    exec 9>/tmp/dnbox-gate-firmware.lock
    flock -x 9
    python "$H/guarded_replay.py" --workspace "$R" --archive "$R/restored-harness-inspect/dot-gate-harness.zip" --elite "$e" --elite-sha "$(sha256sum "$e"|cut -d' ' -f1)" --pro "$p" --pro-sha "$(sha256sum "$p"|cut -d' ' -f1)" --run --offsets=-1 > "$d/s1-ladder-cold.log"
  )
  W=$(head -n1 "$d/s1-ladder-cold.log"); [[ $W == /tmp/dnbox-emulator-only-* ]]
  for m in elite pro; do
    PYTHONPATH="$H" python "$S/seed.py" "$W/$m-virtual-nor.bin" "$d/tree/joint-diagnostic-later/$m/never-install.elf" "$W/$m-offset--1/ram.bin" "$O/tree/build/smk37-elite/x0x.elf" "$O/loaded-smoke-elite/ram.bin" "$S/fixture" "$W/$m-two-owners.bin"
  done
  cp "$d/s1-ladder-cold.log" "$d/cold.log"
  bash "$d/run-actual.sh"
done
```

### replay-elite.json

```json
{"worst_us": 23720, "halves": 423, "raw_late": 192, "xruns": 192, "skipped_cycles": 0, "peak": 13142, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1814, "finished": 1813, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 481, "b": 481}, "joint_audible_spans": 481, "owners": {"count": 5, "owners": [{"ms": 2719, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3182, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4123, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5677, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7834, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### replay-pro.json

```json
{"worst_us": 26090, "halves": 411, "raw_late": 163, "xruns": 163, "skipped_cycles": 0, "peak": 13111, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1761, "finished": 1760, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 453, "b": 453}, "joint_audible_spans": 453, "owners": {"count": 5, "owners": [{"ms": 2603, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3238, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4129, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5672, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7820, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### lookup.patch

```diff
--- a/firmware/src/dsp/juno6.c
+++ b/firmware/src/dsp/juno6.c
@@ -204,6 +204,9 @@
 }
 
 #if X0X_SUPERJ6_FACTORY
+#if !defined(X0X_HOST) && (X0X_SJ6_ALT3_NATIVE || X0X_SJ6_NATIVE_ROOM)
+const float sj6_native_sync_rate_grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
+#endif
 #include "juno6_factory.inc"
 #endif
 
--- a/firmware/src/dsp/juno6_factory.inc
+++ b/firmware/src/dsp/juno6_factory.inc
@@ -701,11 +701,9 @@
     return (float)(((double)tempo/60.0*grid[index])*.25);
 #else
     /* Native PI32v2 has no floating-point widening runtime. */
-    static const float grid[18]={
-        64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f
-    };
+    extern const float sj6_native_sync_rate_grid[18];
     const unsigned index=(unsigned)((1.f-normalized)*17.f+.5f);
-    return (tempo*(1.f/240.f))*grid[index];
+    return (tempo*(1.f/240.f))*sj6_native_sync_rate_grid[index];
 #endif
 }
 int juno6_arm_alt3_running(juno6_t *s,unsigned verified_ramp_frames,int tempo_sync)
--- a/firmware/src/dsp/sj6_native_room.inc
+++ b/firmware/src/dsp/sj6_native_room.inc
@@ -12,9 +12,13 @@
 float sj6_room_rate_clocked(unsigned word,float bpm,int sync)
 {
     if(!sync)return sj6_room_rate(word);
-    static const float grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
+#ifdef X0X_HOST
+    static const float sj6_native_sync_rate_grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
+#else
+    extern const float sj6_native_sync_rate_grid[18];
+#endif
     const unsigned index=(17u*(127u-word)+63u)/127u;
-    return bpm>0.f ? (bpm*(1.f/240.f))*grid[index] : 0.f;
+    return bpm>0.f ? (bpm*(1.f/240.f))*sj6_native_sync_rate_grid[index] : 0.f;
 }
 void original_runtime_sj6_room_clock(original_runtime_part *p,float bpm,int sync)
 {
```

### tables-proof.c

```c
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#define fm_powf powf
const float sj6_native_sync_rate_grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
static float old_sj6_alt3_rate(float normalized,int synced,float tempo)
{
    if (!synced) return 16.0f*fm_powf(normalized,3.33f);
#ifdef X0X_HOST
    static const double grid[18]={
        64,48,32,24,16,12,8,6,4,3,2,1.5,1,.5,.3333,.25,.1667,.125
    };
    const unsigned index=(unsigned)roundf((1.f-normalized)*17.f);
    return (float)(((double)tempo/60.0*grid[index])*.25);
#else
    /* Native PI32v2 has no floating-point widening runtime. */
    static const float grid[18]={
        64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f
    };
    const unsigned index=(unsigned)((1.f-normalized)*17.f+.5f);
    return (tempo*(1.f/240.f))*grid[index];
#endif
}
float old_sj6_room_rate(unsigned word)
{
    float u=(float)word*(1.f/127.f);
    /* Original free rate, authenticated Build9 conductor ID22. */
    return 16.f*fm_powf(u,3.33f);
}
float old_sj6_room_rate_clocked(unsigned word,float bpm,int sync)
{
    if(!sync)return old_sj6_room_rate(word);
    static const float grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
    const unsigned index=(17u*(127u-word)+63u)/127u;
    return bpm>0.f ? (bpm*(1.f/240.f))*grid[index] : 0.f;
}
static float new_sj6_alt3_rate(float normalized,int synced,float tempo)
{
    if (!synced) return 16.0f*fm_powf(normalized,3.33f);
#ifdef X0X_HOST
    static const double grid[18]={
        64,48,32,24,16,12,8,6,4,3,2,1.5,1,.5,.3333,.25,.1667,.125
    };
    const unsigned index=(unsigned)roundf((1.f-normalized)*17.f);
    return (float)(((double)tempo/60.0*grid[index])*.25);
#else
    /* Native PI32v2 has no floating-point widening runtime. */
    extern const float sj6_native_sync_rate_grid[18];
    const unsigned index=(unsigned)((1.f-normalized)*17.f+.5f);
    return (tempo*(1.f/240.f))*sj6_native_sync_rate_grid[index];
#endif
}
float new_sj6_room_rate(unsigned word)
{
    float u=(float)word*(1.f/127.f);
    /* Original free rate, authenticated Build9 conductor ID22. */
    return 16.f*fm_powf(u,3.33f);
}
float new_sj6_room_rate_clocked(unsigned word,float bpm,int sync)
{
    if(!sync)return new_sj6_room_rate(word);
#ifdef X0X_HOST
    static const float sj6_native_sync_rate_grid[18]={64,48,32,24,16,12,8,6,4,3,2,1.5f,1,.5f,.3333f,.25f,.1667f,.125f};
#else
    extern const float sj6_native_sync_rate_grid[18];
#endif
    const unsigned index=(17u*(127u-word)+63u)/127u;
    return bpm>0.f ? (bpm*(1.f/240.f))*sj6_native_sync_rate_grid[index] : 0.f;
}
static int exact(float a,float b){uint32_t x,y;memcpy(&x,&a,4);memcpy(&y,&b,4);return x==y;}
int main(void) {
 unsigned samples=0;
 for(unsigned word=0;word<128;word++)for(int sync=0;sync<2;sync++)
 for(unsigned k=0;k<4096;k++){
  float bpm=((int)k-4)*.0625f, x=(float)word*(1.f/127.f);
  assert(exact(old_sj6_alt3_rate(x,sync,bpm),new_sj6_alt3_rate(x,sync,bpm)));
  assert(exact(old_sj6_room_rate_clocked(word,bpm,sync),new_sj6_room_rate_clocked(word,bpm,sync)));
  samples+=2;
 }
 printf("%u signed float words exact\n",samples);
}
```

### later-elite.json

```json
{
  "elf_sha256": "b3b6d09d780918d729bb09770c4bbed481982bbfeaa3f87d52e2cdd0bc090b83",
  "ro_objects": 381,
  "s1": [
    [
      "bars64",
      ".text",
      "0x2081086",
      38
    ],
    [
      "pref_desc.s1_env_lfo_names",
      ".text",
      "0x207e670",
      16
    ],
    [
      "rd_panel_s1_ref.fixed",
      ".text",
      "0x2084cec",
      69
    ],
    [
      "s1_pan_boundary_hz",
      ".lab_fdata",
      "0x20ed924",
      508
    ],
    [
      "s1_pan_detent_hz",
      ".lab_fdata",
      "0x20ed724",
      512
    ],
    [
      "s1_part_mod_wheel.factor",
      ".text",
      "0x207fe68",
      28
    ],
    [
      "s1_phase_value.defaults",
      ".text",
      "0x207eb9c",
      16
    ],
    [
      "s1_phase_value.lo",
      ".text",
      "0x207eb7c",
      16
    ],
    [
      "s1_phase_value.span",
      ".text",
      "0x207eb8c",
      16
    ],
    [
      "s1_scale",
      ".text",
      "0x208526b",
      42
    ],
    [
      "s1_scale_names",
      ".lab_slot3",
      "0x20b4d70",
      113
    ],
    [
      "s1_scale_ratio",
      ".lab_slot3",
      "0x20b4c08",
      360
    ],
    [
      "s1c_dict",
      ".text",
      "0x207ff24",
      280
    ],
    [
      "s1c_iinit",
      ".lab_fittab",
      "0x20fa3a4",
      150
    ],
    [
      "s1c_imax",
      ".text",
      "0x208540c",
      150
    ],
    [
      "s1c_imin",
      ".text",
      "0x2085376",
      150
    ],
    [
      "s1c_smid",
      ".text",
      "0x2085295",
      46
    ],
    [
      "sin_q",
      ".lab_fittab",
      "0x20fa020",
      260
    ]
  ],
  "sj6": [
    [
      "compact_miami_factory_codes",
      ".lab_fittab",
      "0x20fac40",
      480
    ],
    [
      "juno6_unison_config.graph_gain",
      ".text",
      "0x207fbc4",
      24
    ],
    [
      "native_delay_frames.grid128",
      ".text",
      "0x2081052",
      36
    ],
    [
      "original_chicago_factory_codes",
      ".lab_slot3",
      "0x20b4de1",
      2005
    ],
    [
      "original_chicago_factory_dictionary",
      ".lab_slot3",
      "0x20b55b8",
      1236
    ],
    [
      "original_chicago_factory_name_offset",
      ".text",
      "0x2080f5c",
      162
    ],
    [
      "original_chicago_factory_names",
      ".lab_slot3",
      "0x20b5a8c",
      879
    ],
    [
      "original_sound_chicago_factories",
      ".text",
      "0x2085303",
      81
    ],
    [
      "original_sound_juno_params",
      ".lab_slot3",
      "0x20b6930",
      216
    ],
    [
      "original_sound_sj6_lfo_words.exact",
      ".text",
      "0x207ff14",
      16
    ],
    [
      "rd_panel_sj6_ref.fixed",
      ".lab_slot3",
      "0x20b6d0c",
      312
    ],
    [
      "s1_part_mod_wheel.factor",
      ".text",
      "0x207fe68",
      28
    ],
    [
      "sj6_alt3_rate.grid",
      ".text",
      "0x207fb68",
      72
    ],
    [
      "sj6_aux_route_labels",
      ".lab_slot3",
      "0x20b6bf8",
      276
    ],
    [
      "sj6_knob_desc.delay.1038",
      ".text",
      "0x207e074",
      48
    ],
    [
      "sj6_knob_desc.knobs",
      ".lab_slot3",
      "0x20b6738",
      504
    ],
    [
      "sj6_knob_desc.lfo_wave",
      ".text",
      "0x207e5c8",
      20
    ],
    [
      "sj6_knob_desc.seqcfg",
      ".text",
      "0x207e020",
      36
    ],
    [
      "sj6_knob_desc.sources",
      ".text",
      "0x207e044",
      24
    ],
    [
      "sj6_knob_desc.toggle",
      ".text",
      "0x207e05c",
      24
    ],
    [
      "sj6_knob_desc.trem",
      ".text",
      "0x207e008",
      24
    ],
    [
      "sj6_knob_desc.wave.1039",
      ".text",
      "0x207e5e4",
      16
    ],
    [
      "sj6_normal_cutoff_hz.hi",
      ".text",
      "0x207fbb0",
      20
    ],
    [
      "sj6_sub_harmonic",
      ".text",
      "0x20851d7",
      48
    ],
    [
      "sj6_unison_coeff.pan",
      ".text",
      "0x207fbdc",
      20
    ],
    [
      "sj6_velocity_scale",
      ".lab_fdata",
      "0x20edb20",
      512
    ],
    [
      "ui_original_name.juno",
      ".text",
      "0x207ee4c",
      64
    ],
    [
      ".Lsj6_run_modulated_effects.mod",
      ".text",
      "0x20808b4",
      20
    ],
    [
      "sj6_limiter_unit.defaults",
      ".text",
      "0x2080858",
      16
    ],
    [
      "sj6_midi_velocity",
      ".lab_fittab",
      "0x20fa224",
      384
    ],
    [
      "sj6_room_rate_clocked.grid",
      ".text",
      "0x2080810",
      72
    ],
    [
      "source_factor_bits.f",
      ".text",
      "0x2080868",
      76
    ],
    [
      "sj6_aux_destination_ids",
      ".text",
      "0x208520a",
      78
    ]
  ],
  "exact_ro_duplicate": [
    [
      [
        "IMA_ROUND_CORR",
        ".text",
        "0x2085a05",
        64
      ],
      [
        "IMA_ROUND_CORR.1385",
        ".text",
        "0x20852c3",
        64
      ]
    ],
    [
      [
        ".Lswitch.table.1669",
        ".text",
        "0x207da10",
        48
      ],
      [
        ".Lswitch.table.1670",
        ".text",
        "0x207da40",
        48
      ],
      [
        ".Lswitch.table.1671",
        ".text",
        "0x207da70",
        48
      ],
      [
        ".Lswitch.table.1672",
        ".text",
        "0x207daa0",
        48
      ]
    ],
    [
      [
        "chi_bpmdly_beats.beats",
        ".text",
        "0x207f844",
        52
      ],
      [
        "fx_beats",
        ".text",
        "0x20806c0",
        52
      ]
    ],
    [
      [
        "describe_pages.refs.124",
        ".text",
        "0x20845d5",
        16
      ],
      [
        "describe_pages.refs.126",
        ".text",
        "0x2084615",
        16
      ]
    ],
    [
      [
        "sj6_alt3_rate.grid",
        ".text",
        "0x207fb68",
        72
      ],
      [
        "sj6_room_rate_clocked.grid",
        ".text",
        "0x2080810",
        72
      ]
    ]
  ],
  "cross_array_substrings": [],
  "intersection_distinct_word_count": 28
}
```

### later-pro.json

```json
{
  "elf_sha256": "a005f3525549073f5a8e6c9c450d44c0cfdf83e804a1437b281438c1748acfef",
  "ro_objects": 380,
  "s1": [
    [
      "bars64",
      ".text",
      "0x2081082",
      38
    ],
    [
      "pref_desc.s1_env_lfo_names",
      ".text",
      "0x207e670",
      16
    ],
    [
      "rd_panel_s1_ref.fixed",
      ".text",
      "0x2084ce2",
      69
    ],
    [
      "s1_pan_boundary_hz",
      ".lab_fdata",
      "0x20ed924",
      508
    ],
    [
      "s1_pan_detent_hz",
      ".lab_fdata",
      "0x20ed724",
      512
    ],
    [
      "s1_part_mod_wheel.factor",
      ".text",
      "0x207fe68",
      28
    ],
    [
      "s1_phase_value.defaults",
      ".text",
      "0x207eb9c",
      16
    ],
    [
      "s1_phase_value.lo",
      ".text",
      "0x207eb7c",
      16
    ],
    [
      "s1_phase_value.span",
      ".text",
      "0x207eb8c",
      16
    ],
    [
      "s1_scale",
      ".text",
      "0x2085261",
      42
    ],
    [
      "s1_scale_names",
      ".lab_slot3",
      "0x20b4d70",
      113
    ],
    [
      "s1_scale_ratio",
      ".lab_slot3",
      "0x20b4c08",
      360
    ],
    [
      "s1c_dict",
      ".text",
      "0x207ff24",
      280
    ],
    [
      "s1c_iinit",
      ".lab_fittab",
      "0x20fa3a4",
      150
    ],
    [
      "s1c_imax",
      ".text",
      "0x2085402",
      150
    ],
    [
      "s1c_imin",
      ".text",
      "0x208536c",
      150
    ],
    [
      "s1c_smid",
      ".text",
      "0x208528b",
      46
    ],
    [
      "sin_q",
      ".lab_fittab",
      "0x20fa020",
      260
    ]
  ],
  "sj6": [
    [
      "compact_miami_factory_codes",
      ".lab_fittab",
      "0x20fac40",
      480
    ],
    [
      "juno6_unison_config.graph_gain",
      ".text",
      "0x207fbc4",
      24
    ],
    [
      "native_delay_frames.grid128",
      ".text",
      "0x208104e",
      36
    ],
    [
      "original_chicago_factory_codes",
      ".lab_slot3",
      "0x20b4de1",
      2005
    ],
    [
      "original_chicago_factory_dictionary",
      ".lab_slot3",
      "0x20b55b8",
      1236
    ],
    [
      "original_chicago_factory_name_offset",
      ".text",
      "0x2080f58",
      162
    ],
    [
      "original_chicago_factory_names",
      ".lab_slot3",
      "0x20b5a8c",
      879
    ],
    [
      "original_sound_chicago_factories",
      ".text",
      "0x20852f9",
      81
    ],
    [
      "original_sound_juno_params",
      ".lab_slot3",
      "0x20b6930",
      216
    ],
    [
      "original_sound_sj6_lfo_words.exact",
      ".text",
      "0x207ff14",
      16
    ],
    [
      "rd_panel_sj6_ref.fixed",
      ".lab_slot3",
      "0x20b6d0c",
      312
    ],
    [
      "s1_part_mod_wheel.factor",
      ".text",
      "0x207fe68",
      28
    ],
    [
      "sj6_alt3_rate.grid",
      ".text",
      "0x207fb68",
      72
    ],
    [
      "sj6_aux_route_labels",
      ".lab_slot3",
      "0x20b6bf8",
      276
    ],
    [
      "sj6_knob_desc.delay.1038",
      ".text",
      "0x207e074",
      48
    ],
    [
      "sj6_knob_desc.knobs",
      ".lab_slot3",
      "0x20b6738",
      504
    ],
    [
      "sj6_knob_desc.lfo_wave",
      ".text",
      "0x207e5c8",
      20
    ],
    [
      "sj6_knob_desc.seqcfg",
      ".text",
      "0x207e020",
      36
    ],
    [
      "sj6_knob_desc.sources",
      ".text",
      "0x207e044",
      24
    ],
    [
      "sj6_knob_desc.toggle",
      ".text",
      "0x207e05c",
      24
    ],
    [
      "sj6_knob_desc.trem",
      ".text",
      "0x207e008",
      24
    ],
    [
      "sj6_knob_desc.wave.1039",
      ".text",
      "0x207e5e4",
      16
    ],
    [
      "sj6_normal_cutoff_hz.hi",
      ".text",
      "0x207fbb0",
      20
    ],
    [
      "sj6_sub_harmonic",
      ".text",
      "0x20851cd",
      48
    ],
    [
      "sj6_unison_coeff.pan",
      ".text",
      "0x207fbdc",
      20
    ],
    [
      "sj6_velocity_scale",
      ".lab_fdata",
      "0x20edb20",
      512
    ],
    [
      "ui_original_name.juno",
      ".text",
      "0x207ee4c",
      64
    ],
    [
      ".Lsj6_run_modulated_effects.mod",
      ".text",
      "0x20808b4",
      20
    ],
    [
      "sj6_limiter_unit.defaults",
      ".text",
      "0x2080858",
      16
    ],
    [
      "sj6_midi_velocity",
      ".lab_fittab",
      "0x20fa224",
      384
    ],
    [
      "sj6_room_rate_clocked.grid",
      ".text",
      "0x2080810",
      72
    ],
    [
      "source_factor_bits.f",
      ".text",
      "0x2080868",
      76
    ],
    [
      "sj6_aux_destination_ids",
      ".text",
      "0x2085200",
      78
    ]
  ],
  "exact_ro_duplicate": [
    [
      [
        "IMA_ROUND_CORR",
        ".text",
        "0x20859fb",
        64
      ],
      [
        "IMA_ROUND_CORR.1385",
        ".text",
        "0x20852b9",
        64
      ]
    ],
    [
      [
        ".Lswitch.table.1669",
        ".text",
        "0x207da10",
        48
      ],
      [
        ".Lswitch.table.1670",
        ".text",
        "0x207da40",
        48
      ],
      [
        ".Lswitch.table.1671",
        ".text",
        "0x207da70",
        48
      ],
      [
        ".Lswitch.table.1672",
        ".text",
        "0x207daa0",
        48
      ]
    ],
    [
      [
        "chi_bpmdly_beats.beats",
        ".text",
        "0x207f844",
        52
      ],
      [
        "fx_beats",
        ".text",
        "0x20806c0",
        52
      ]
    ],
    [
      [
        "describe_pages.refs.124",
        ".text",
        "0x20845cb",
        16
      ],
      [
        "describe_pages.refs.126",
        ".text",
        "0x208460b",
        16
      ]
    ],
    [
      [
        "sj6_alt3_rate.grid",
        ".text",
        "0x207fb68",
        72
      ],
      [
        "sj6_room_rate_clocked.grid",
        ".text",
        "0x2080810",
        72
      ]
    ]
  ],
  "cross_array_substrings": [],
  "intersection_distinct_word_count": 28
}
```
