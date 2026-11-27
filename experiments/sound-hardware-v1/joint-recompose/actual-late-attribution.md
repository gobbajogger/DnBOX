# Joint co-audible actual DMA-miss attribution
Pinned native-ed-brk-recompose source. This is a diagnostic only: enable
DNBOX_LAB_JOINT_HALF_ATTR=1 with prior verified fixed typed cue and plan4.
Stage script refuses any edited donor via exact engine.c/main_fm1.c hashes.
No emitter/pin/DSP, producer cue, app feature or existing late gate change.

The existing lab_bass_half counter measures Core0 only. Five new POOL words
outside normal Core0 BSS capture actual 909 C0 active job elapsed (exclusive
of any recorded wait), A and B before their ordered mix wait, and each
explicit ordered wait. Reuses already scheduled tick measurements and
existing co-audible span counter: no new clock reads and no PCM scans.
Cycle counts are timer ticks, not pure instruction counts; nested IRQ can
contribute and Core0/C1 intervals overlap. Commit, master, post-FX and 808
prefix are not attributed. Hardware-fault/takeover replay invalidates the
timing characterization and must be rejected via same run status guards.

At ALNK0 after its existing physical moved-half boolean, record first and
worst actual late-coaudible half, sums across every such half, and separate
counts for requested 10809-us threshold and actual DMA movement. Worst
joint-late need not be global worst. Audible owner marks mean at least one
non-silent A sample and one B sample in the same span, not same-sample
overlap. Footer itself adds overhead outside x0x_dbg.last_us timestamp, so
comparison vs an uninstrumented run is not a timing improvement claim.

read.py linked.elf exact-shot-directory expected-elf-sha decodes stopped
data, checking generation/boot counters, native no-skip and five owner
replies. This does not qualify USB/MIDI producer frame alignment. Keep the
full cold seal/assistant gate and serialized emulator flock rules.

## Paired native replay

All 168 Elite and 169 Pro DMA misses are on halves with both engines confirmed audible. Across those misses, Synth One averaged ~12.3 ms of pre-wait work on each model; the worst co-audible halves featured ~17.8 ms Elite / 16.8 ms Pro of SuperJ6 pre-wait elapsed work alone. The measured B ordered waits in those two worst halves were only 0.54 µs. Average role duration excludes some downstream work; overlap, nested interrupts and instrumentation mean these are not additive or an exact CPU budget. The test establishes that waiting or decoder caching alone is not a fix. Both models failed timing with authentic concurrent playback, five settled typed owner replies, zero skipped cycles and unchanged virtual NOR. No production change.

### Elite

```json
{"all_halves": 413, "actual_late": 168, "joint_halves": 194, "joint_late": 168, "joint_above_10809_us": 191, "labels": ["909-C0-exclusive-stage", "S1-A-prewait", "SJ6-B-prewait", "A-ordered-wait", "B-ordered-wait"], "mean_late_role_us": [4787.26, 12321.2, 7237.81, 0.65, 1423.44], "first": {"half": 224, "us": 11709, "coaudible_same_span_count": 2, "role_us": [3258.79, 10215.04, 4407.0, 0.0, 2647.54]}, "worst": {"half": 291, "us": 22716, "coaudible_same_span_count": 2, "role_us": [3282.17, 12822.54, 17790.29, 0.0, 0.54]}, "caveat": "Diagnostic. Includes nested IRQ in clocks, excludes commit/master/FX; role costs on both cores overlap. Worst among actual late joint-audible halves, not necessarily global worst."}
```

### Pro

```json
{"all_halves": 412, "actual_late": 169, "joint_halves": 193, "joint_late": 169, "joint_above_10809_us": 191, "labels": ["909-C0-exclusive-stage", "S1-A-prewait", "SJ6-B-prewait", "A-ordered-wait", "B-ordered-wait"], "mean_late_role_us": [4999.98, 12335.58, 6981.22, 1.03, 1376.65], "first": {"half": 222, "us": 12434, "coaudible_same_span_count": 2, "role_us": [3804.96, 10828.17, 4440.58, 0.0, 2676.17]}, "worst": {"half": 286, "us": 22667, "coaudible_same_span_count": 2, "role_us": [4230.79, 12536.38, 16808.29, 0.0, 0.54]}, "caveat": "Diagnostic. Includes nested IRQ in clocks, excludes commit/master/FX; role costs on both cores overlap. Worst among actual late joint-audible halves, not necessarily global worst."}
```

### Diagnostic patch

```diff
--- a/firmware/src/app/engine.c
+++ b/firmware/src/app/engine.c
@@ -3011,6 +3011,15 @@
 static uint32_t eng_job_arg;                           /* this stretch's n, 808 mask and window offset */
 static uint8_t eng_job_solo;
 static uint8_t eng_c1_waited;                          /* core 0 has called plat_cpu2_wait this stretch */
+#if DNBOX_LAB_JOINT_HALF_ATTR
+#if !DNBOX_LAB_JOINT_FIXED_RENDER_CUE || X0X_CORE_PLAN!=4
+#error exact joint typed plan4 witness required
+#endif
+/* Core1 may write POOL, not to the old C0-only .lab_bass counter. Each
+ * index single job owner; C0 snapshots only after the real worker join.
+ * Order: 909 execution, A pre-wait, B pre-wait, A/B explicit wait. */
+volatile uint32_t lab_joint_half_ticks[5] POOL;
+#endif
 static uint8_t eng_brk_mark;                           /* brk_midi_count before core 1 could add any */
 /* same-block 808 split: BD+SD (a prefix of the mix, about 1.2 ms) on core 1,
  * metals' voices on core 0 in parallel, mid lanes and metal mix after the body
@@ -3245,9 +3254,15 @@
         w0 = plat_ticks24();
         extern volatile uint32_t lab_bass_half[2];
         if (!core) lab_bass_half[bv] += w0 - tk;
+#if DNBOX_LAB_JOINT_HALF_ATTR
+        lab_joint_half_ticks[1+bv] += w0-tk;
+#endif
         if (stage_wait((uint32_t)(2 + bv), core))         /* A mixes after the 808, B after A */
             return 0;
         wt = plat_ticks24() - w0;
+#if DNBOX_LAB_JOINT_HALF_ATTR
+        lab_joint_half_ticks[3+bv] += wt;
+#endif
         if (status == ORIGINAL_RUNTIME_OK) {
             /* A part that was silent when Solo excluded it must not leak a new
              * note through the 10 ms mix glide-out: start its glides at zero.
@@ -3286,8 +3301,16 @@
         if (!core)
             PROF_ADD(PR_BRK, pk);
     }
+#if DNBOX_LAB_JOINT_HALF_ATTR
+    if (active) {
+        const uint32_t elapsed=plat_ticks24()-tk-wt;
+        job_cost(j,elapsed,n);
+        if(!core && j==JB_909) lab_joint_half_ticks[0]+=elapsed;
+    }
+#else
     if (active)
         job_cost(j, plat_ticks24() - tk - wt, n);
+#endif
     return 1;
 }
 
--- a/firmware/src/app/main_fm1.c
+++ b/firmware/src/app/main_fm1.c
@@ -101,6 +101,43 @@
     x0x_dbg.in_audio = 0;
 }
 
+#if DNBOX_LAB_JOINT_HALF_ATTR
+extern volatile uint32_t lab_joint_half_ticks[5];
+extern volatile uint32_t lab_joint_fixed_audible_spans;
+static uint32_t lab_joint_at_before;
+struct lab_joint_late_half {
+    uint32_t half,us,coaudible_spans,ticks[5];
+};
+volatile struct lab_joint_late_stats {
+    uint32_t all_halves,actual_late,joint_halves,joint_late,joint_above_budget;
+    uint32_t late_sum_ticks[5];
+    struct lab_joint_late_half first_joint_late,worst_joint_late;
+} lab_joint_late_stats;
+static inline void lab_joint_snapshot(volatile struct lab_joint_late_half *row,uint32_t spans)
+{
+    row->half=x0x_dbg.halves;row->us=x0x_dbg.last_us;row->coaudible_spans=spans;
+    for(unsigned j=0;j<5;++j)row->ticks[j]=lab_joint_half_ticks[j];
+}
+__attribute__((noinline,minsize)) static void lab_joint_half_done(int late)
+{
+    const uint32_t now=lab_joint_fixed_audible_spans, n=now-lab_joint_at_before;
+    lab_joint_at_before=now;
+    ++lab_joint_late_stats.all_halves;
+    lab_joint_late_stats.actual_late+=late;
+    if(n){
+        ++lab_joint_late_stats.joint_halves;
+        lab_joint_late_stats.joint_above_budget+=x0x_dbg.last_us>10809u;
+        if(late){
+            ++lab_joint_late_stats.joint_late;
+            if(lab_joint_late_stats.joint_late==1)lab_joint_snapshot(&lab_joint_late_stats.first_joint_late,n);
+            if(x0x_dbg.last_us>lab_joint_late_stats.worst_joint_late.us)
+                lab_joint_snapshot(&lab_joint_late_stats.worst_joint_late,n);
+            for(unsigned j=0;j<5;++j)lab_joint_late_stats.late_sum_ticks[j]+=lab_joint_half_ticks[j];
+        }
+    }
+    for(unsigned j=0;j<5;++j)lab_joint_half_ticks[j]=0;
+}
+#endif
 void fm1_alnk0_irq(void)                            /* via isr_alnk0 (hal/fm1_isr.S) */
 {
     uint8_t p = fm1_audio_pending();
@@ -109,13 +146,21 @@
         uint32_t half = fm1_audio_free_half();
         render_block(&abuf[half * HALF_WORDS]);
         fm1_audio_ack_half();
+#if DNBOX_LAB_JOINT_HALF_ATTR
+        const int late = fm1_audio_free_half() != half;
+        if(late) {
+#else
         if (fm1_audio_free_half() != half) {
+#endif
 #if X0X_DEBUG
             x0x_worst.late_in++;
 #endif
             x0x_dbg.late++;                         /* the DMA moved on while we rendered */
             audio_xruns++;
         }
+#if DNBOX_LAB_JOINT_HALF_ATTR
+        lab_joint_half_done(late); /* after the one real DMA late decision */
+#endif
     }
 }
 extern void isr_alnk0(void);
```

### Pinned RAM reader

```python
#!/usr/bin/env python3
import sys,struct,json
from pathlib import Path
r=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(r/'restored-harness-inspect/helpers'))
import guarded_replay as g
elf,shot=map(Path,sys.argv[1:3])
raw,sym,_=g.symbols_and_sections(elf)
assert g.sha(raw)==sys.argv[3]
ram=(shot/'ram.bin').read_bytes()
def unpack(label,n):
    off=sym[label]-g.ELF_RAM
    assert 0<=off<=len(ram)-4*n
    return struct.unpack_from('<'+'I'*n,ram,off)
allh,late,joint,jlate,budget,*entries=unpack('lab_joint_late_stats',26)
dbg=unpack('x0x_dbg',13)
assert (allh,late)==(dbg[1],dbg[5])
assert 0<=jlate<=joint<=allh and 0<=jlate<=late<=allh and budget<=joint
result=json.loads((shot/'result.json').read_bytes())
assert result['sealed_boot'] and result['skipped_cycles']==0
assert result['settled_owner_readbacks']==5 and result['core1']['alive']
assert not (result['core1']['bad_mailbox'] or result['core1']['gave_up'])
def parse(e):
    h,us,n,*ticks=e
    assert 0<h<=allh and n>0 and us>0
    return dict(half=h,us=us,coaudible_same_span_count=n,role_us=[round(x/24,2) for x in ticks])
first,worst=[parse(entries[i:i+8]) if jlate else None for i in (5,13)]
assert not first or first['half']<=worst['half'] and first['us']<=worst['us']
print(json.dumps(dict(all_halves=allh,actual_late=late,joint_halves=joint,joint_late=jlate,
    joint_above_10809_us=budget,labels=['909-C0-exclusive-stage','S1-A-prewait','SJ6-B-prewait','A-ordered-wait','B-ordered-wait'],
    mean_late_role_us=[round(x/(jlate*24),2) if jlate else None for x in entries[:5]],first=first,worst=worst,
    caveat='Diagnostic. Includes nested IRQ in clocks, excludes commit/master/FX; role costs on both cores overlap. Worst among actual late joint-audible halves, not necessarily global worst.')))
```

### Native link receipts

```text
guarded elite XIP 552284 free 388 pool usable 2568 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000001000000d8ef00007def59efcc1c64c995a0ba05575691cf ELF c3709a813ef23401f560b8d98a74832070cf722b6226e0c99221765c5c043ea6
guarded pro XIP 552272 free 400 pool usable 2568 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000002000000d8ef00007def59ef95ecdf4b1ea0386097cb627c ELF f2df021ce155c0f70a72fe56054b952e418f8f84c7e743fe8f92fbfb60726cc8
```
