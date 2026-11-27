# Joined SuperJ6 routing and memory diagnostic: still not release-ready

Exact baseline c24f2f2f paired recompose. Apply independently verified checkpoint patches in this order using `patch --fuzz=0 -p1`:
1. [Type II Room parallel return](../sj6-stage-ab/room/parallel-return.md).
2. [Active Delay history retime](../sj6-stage-ab/delay/live-retime.md).
3. [909 scratch reclaim and foreground list relocation, then exact sync-grid dedup](909-scratch-sync-dedup.md).

Joined native links **540 B Elite / 552 B Pro free APP**, **3,236 B usable pool** after reserve both; independent deltas are now confirmed together. Both synths were actually audible (476 / 453 joint span witnesses), 5/5 typed-owner readbacks, zero skips and bad mailboxes, stock virtual NOR unchanged. Still **190 Elite / 167 Pro XRUNs**; worst 23,669 / 27,233 µs against ≤10,809 and zero. This default loaded cue is not signed active FX parity or a runtime proof of the Type II/retime effect transitions. No production or real hardware.

## Separate non-adopted Synth One morph optimization

Avoid recomputing both waveform shape/fraction decodes for every voice; lazy per-sample shared decode retains operation order and signed state. Host exactly matched 576,672 signed stereo frames plus voice/RNG state in internal and external ARP configurations. Native unjoined link requires 192 B app on each board (596/608 B remain). Same candidate replay: Elite 26,041 µs/163 XRUNs with 453 both-audible spans and 5/5 owner readbacks. Pro 23,646 µs/194 XRUNs with 477 both-audible spans but only 4/5 readbacks; strictly failed witness. Neither clears zero dropouts; not joined or promoted. This is separate from the LP optimization and there is no sound A/B claim here.

Continue from this evidence, not the rejected LP/morph hot experiments, low-clock Room or fitted gains. Open sound work remains the observed shared dry magnitude and exact Sub clock ingress, steady Delay bandwidth and actual source Room response on dense and heldout inputs.


### sj6-parallel-joined/join.log

```text
519aaa0d148ffa93ebc9c6ef61e9fe74374b86cfbf7bbe18a534c8b0983b2058  sj6-room-state-next/mix/parallel-return.patch
patching file firmware/src/dsp/sj6_native_room.inc
d3583fd336dcba0b3e09443e9c0eb1e94181b5e60b84a48e97a009043fdb355e  sj6-delay-live-next/retime.patch
patching file firmware/src/dsp/juno6_delay_native.inc
cbf25213bdae3a3d196408d000e008001af4f90d64c88dc1dd9c1bc2c53ae34e  firmware-fit-next/mono-relocate.patch
patching file firmware/src/app/engine.c
patching file firmware/src/app/ui.c
dfa54e8e89671c5c5ef9d5c003017d74434bdebe2ca22e28d2d30b38de91794a  /tmp/s1-lookup-dedup.xDXTzkSx/lookup.patch
patching file firmware/src/dsp/juno6.c
patching file firmware/src/dsp/juno6_factory.inc
patching file firmware/src/dsp/sj6_native_room.inc
```

### sj6-parallel-joined/run-native-check.sh

```bash
#!/usr/bin/env bash
set -euo pipefail
R=$(cd "$(dirname "$0")/.." && pwd)
S="$R/s1-organ-wet-recovery/native-ed-brk-recompose"
H="$R/restored-harness-inspect/helpers"
O="$R/remote-typed-load"
for d in "$R/sj6-parallel-joined"; do
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

### sj6-parallel-joined/replay-elite.json

```json
{"worst_us": 23669, "halves": 421, "raw_late": 190, "xruns": 190, "skipped_cycles": 0, "peak": 13151, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1805, "finished": 1804, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 476, "b": 476}, "joint_audible_spans": 476, "owners": {"count": 5, "owners": [{"ms": 2660, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3229, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4119, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5673, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7833, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### sj6-parallel-joined/replay-pro.json

```json
{"worst_us": 27233, "halves": 411, "raw_late": 167, "xruns": 167, "skipped_cycles": 0, "peak": 13141, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1761, "finished": 1760, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 453, "b": 453}, "joint_audible_spans": 453, "owners": {"count": 5, "owners": [{"ms": 2669, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3204, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4127, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5699, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7866, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### s1-render-next/morph-control/morph-control.patch

```diff
--- a/firmware/src/dsp/s1c/s1c.c
+++ b/firmware/src/dsp/s1c/s1c.c
@@ -345,22 +345,29 @@
     if (bl) { float u = t + 1.0f - w; if (u >= 1.0f) u -= 1.0f; y += blep(u, dt) - blep(t, dt); }
     return y;
 }
-static float morph(uint32_t *ph, float dt, uint32_t increment, float pos, int bl)
+/* Decode the oscillator shape coordinate once per render sample, not once
+ * per active voice. Smoothing and sequencer updates still precede decoding. */
+typedef struct { int k; float f; } s1c_morph_control;
+static inline s1c_morph_control morph_control(float pos)
 {
-    float t = (float)*ph * (1.0f / Q32);
     if (pos > 1.0f) pos -= (int)pos;
     float f = pos * 3.0f; int k = (int)f; if (k > 2) k = 2;
-    f -= k;
-    float y = shape(k, t, dt, bl);
-    if (f > 0.0f) y += f * (shape(k + 1, t, dt, bl) - y);
-    *ph += increment;
+    return (s1c_morph_control){ k, f - k };
+}
+static __attribute__((noinline)) float morph_decoded(uint32_t *ph, float dt, uint32_t inc,
+                                  s1c_morph_control c, int bl)
+{
+    float t = (float)*ph * (1.0f / Q32);
+    float y = shape(c.k, t, dt, bl);
+    if (c.f > 0.0f) y += c.f * (shape(c.k + 1, t, dt, bl) - y);
+    *ph += inc;
     return y;
 }
 #if X0X_SYNTHONE_PATCH
 /* Panel preview uses the same continuous morph with the note-dependent
  * antialiasing disabled. It does not advance any playing oscillator. */
 float s1c_morph_preview(float position, uint32_t phase)
-{ return morph(&phase, 0.f, 0, position, 0); }
+{ return morph_decoded(&phase, 0.f, 0, morph_control(position), 0); }
 #endif
 static float sine_q(uint32_t ph) { return sinf((float)ph * (TWO_PI / Q32)); }
 
@@ -451,8 +458,8 @@
     cut = clampf(cut - cut * em * (1.0f - v->fenv), 64.0f, 22050.0f);
     int bl = P(BLE) > 0.0f;
     float dt1 = f1 / s->sr, dt2 = f2 / s->sr;
-    float o1 = morph(&v->ph[0], dt1, ph_inc(dt1), P(I1), bl) * v->vamp * P(VOL1);
-    float o2 = morph(&v->ph[1], dt2, ph_inc(dt2), P(I2), bl) * v->vamp * P(VOL2);
+    float o1 = morph_decoded(&v->ph[0], dt1, ph_inc(dt1), morph_control(P(I1)), bl) * v->vamp * P(VOL1);
+    float o2 = morph_decoded(&v->ph[1], dt2, ph_inc(dt2), morph_control(P(I2)), bl) * v->vamp * P(VOL2);
     float y = (1.0f - bal) * o1 + bal * o2;
     float sub = sine_q(v->ph[2]);
     v->ph[2] += ph_inc(fs / s->sr);
@@ -483,7 +490,7 @@
 #if S1C_KR > 1
 /* Cheap voice: pitch, LFO, key-tracking and filter coefficients at control rate (every S1C_KR
  * samples), sines from a 256-point table, the filter envelope still per sample in the cutoff. */
-static float voice_run_kr(s1c *s, s1c_voice *v)
+static float voice_run_kr(s1c *s, s1c_voice *v, const s1c_morph_control mc[2], int bl)
 {
     int mono = P(MONO) > 0.0f, ft = (int)P(FTYPE);
     v->amp = env_step(&v->ea, v->gate, P(AA), clampf(P(AD) * lfo10(s, P(LDEC)), 0.005f, 2.0f), P(AS), P(AR), s->sr);
@@ -531,9 +538,8 @@
             v->ka1 = a1; v->ka4 = 2.0f * (cc * cc - 1.0f) * a1; v->ka5 = (1.0f - 1.41421356f * cc + cc * cc) * a1;
         }
     }
-    int bl = P(BLE) > 0.0f;
-    float o1 = morph(&v->ph[0], v->kd1, v->ki1, P(I1), bl) * v->vamp * P(VOL1);
-    float o2 = morph(&v->ph[1], v->kd2, v->ki2, P(I2), bl) * v->vamp * P(VOL2);
+    float o1 = morph_decoded(&v->ph[0], v->kd1, v->ki1, mc[0], bl) * v->vamp * P(VOL1);
+    float o2 = morph_decoded(&v->ph[1], v->kd2, v->ki2, mc[1], bl) * v->vamp * P(VOL2);
     float y = (1.0f - v->kbal) * o1 + v->kbal * o2;
     float sub = sine_t(v->ph[2]);
     v->ph[2] += v->kis;
@@ -583,7 +589,7 @@
     }
     return y + P(FMIX) * (fo - y);
 }
-#define voice_run voice_run_kr
+#define voice_run(s,v) voice_run_kr(s,v,mc,bl)
 #endif
 /* ---- FX ---- */
 typedef s1u_cc s1c_cc;
@@ -1183,8 +1189,21 @@
         sequencer(s);
 #endif
         float y = 0.0f;
-        if (P(MONO) > 0.0f) { if (s->v[0].root != -1 && s->v[0].stage) y = voice_run(s, &s->v[0]); }
-        else for (i = 0; i < S1C_VOICES; i++) if (s->v[i].root != -1 && s->v[i].stage) y += voice_run(s, &s->v[i]);
+#if S1C_KR > 1 || S1C_FAST
+        s1c_morph_control mc[2];
+        int bl = 0;
+        /* Lazy on the first admitted voice. Keep idle/invalid coordinates
+         * uninspected just like the original, and preserve ordered addition. */
+        int decoded = 0;
+#define S1C_PREP_MORPH() do { if (!decoded) { \
+    mc[0] = morph_control(P(I1)); mc[1] = morph_control(P(I2)); \
+    bl = P(BLE) > 0.0f; decoded = 1; } } while (0)
+#else
+#define S1C_PREP_MORPH() ((void)0)
+#endif
+        if (P(MONO) > 0.0f) { if (s->v[0].root != -1 && s->v[0].stage) { S1C_PREP_MORPH(); y = voice_run(s, &s->v[0]); } }
+        else for (i = 0; i < S1C_VOICES; i++) if (s->v[i].root != -1 && s->v[i].stage) { S1C_PREP_MORPH(); y += voice_run(s, &s->v[i]); }
+#undef S1C_PREP_MORPH
 
         /* bitcrush (sample-and-hold), tremolo */
 #if defined(X0X_S1_CRUSH_LFO_PROBE) && X0X_S1_CRUSH_LFO_PROBE
```

### s1-render-next/morph-control/run.sh

```bash
#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
D=$(realpath ../../s1-organ-wet-recovery/native-ed-brk-attribution/tree/firmware/src/dsp/s1c)
python stage.py "$D/s1c.c" s1c.c
for seq in 0 1; do for side in old new; do
H="$D"; [[ $side = old ]] || H="$PWD"
cc -O2 -DX0X_HOST=1 -DX0X_S1C=1 -DS1C_NO_SEQ=$seq -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -ffp-contract=off -ffunction-sections -fdata-sections -I"$H" -I"$D" -I"$D/.." -I"$D/../../../hal" -I"$D/../.." voice-check.c "$D/s1u.c" ../ima-tables.c -Wl,--gc-sections -lm -o "$side-$seq"
"./$side-$seq" > "$side-$seq.raw"
done; cmp old-$seq.raw new-$seq.raw; sha256sum old-$seq.raw new-$seq.raw;done
```

### s1-render-next/morph-control/stage.py

```python
import hashlib, pathlib, sys
source=pathlib.Path(sys.argv[1]); s=source.read_text()
print(hashlib.sha256(source.read_bytes()).hexdigest())
def replace(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
start=s.index('static float morph(uint32_t *ph'); end=s.index('#if X0X_SYNTHONE_PATCH',start)
s=s[:start]+s[end:]
point='#if X0X_SYNTHONE_PATCH\n/* Panel preview'
replace(point,'''/* Decode the oscillator shape coordinate once per render sample, not once
 * per active voice. Smoothing and sequencer updates still precede decoding. */
typedef struct { int k; float f; } s1c_morph_control;
static inline s1c_morph_control morph_control(float pos)
{
    if (pos > 1.0f) pos -= (int)pos;
    float f = pos * 3.0f; int k = (int)f; if (k > 2) k = 2;
    return (s1c_morph_control){ k, f - k };
}
static __attribute__((noinline)) float morph_decoded(uint32_t *ph, float dt, uint32_t inc,
                                  s1c_morph_control c, int bl)
{
    float t = (float)*ph * (1.0f / Q32);
    float y = shape(c.k, t, dt, bl);
    if (c.f > 0.0f) y += c.f * (shape(c.k + 1, t, dt, bl) - y);
    *ph += inc;
    return y;
}
'''+point)
replace('return morph(&phase, 0.f, 0, position, 0);','return morph_decoded(&phase, 0.f, 0, morph_control(position), 0);')
s=s.replace('ph_inc(dt1), P(I1), bl)', 'ph_inc(dt1), morph_control(P(I1)), bl)').replace('ph_inc(dt2), P(I2), bl)', 'ph_inc(dt2), morph_control(P(I2)), bl)')
replace('static float voice_run_kr(s1c *s, s1c_voice *v)','static float voice_run_kr(s1c *s, s1c_voice *v, const s1c_morph_control mc[2], int bl)')
replace('    int bl = P(BLE) > 0.0f;\n    float o1 = morph(&v->ph[0], v->kd1, v->ki1, P(I1), bl) * v->vamp * P(VOL1);\n    float o2 = morph(&v->ph[1], v->kd2, v->ki2, P(I2), bl) * v->vamp * P(VOL2);','    float o1 = morph_decoded(&v->ph[0], v->kd1, v->ki1, mc[0], bl) * v->vamp * P(VOL1);\n    float o2 = morph_decoded(&v->ph[1], v->kd2, v->ki2, mc[1], bl) * v->vamp * P(VOL2);')
s=s.replace("= morph(&v->ph", "= morph_decoded(&v->ph")
replace('#define voice_run voice_run_kr','#define voice_run(s,v) voice_run_kr(s,v,mc,bl)')
replace('        float y = 0.0f;\n        if (P(MONO)', '''        float y = 0.0f;
#if S1C_KR > 1 || S1C_FAST
        s1c_morph_control mc[2];
        int bl = 0;
        /* Lazy on the first admitted voice. Keep idle/invalid coordinates
         * uninspected just like the original, and preserve ordered addition. */
        int decoded = 0;
#define S1C_PREP_MORPH() do { if (!decoded) { \\
    mc[0] = morph_control(P(I1)); mc[1] = morph_control(P(I2)); \\
    bl = P(BLE) > 0.0f; decoded = 1; } } while (0)
#else
#define S1C_PREP_MORPH() ((void)0)
#endif
        if (P(MONO)''')
replace('&& s->v[0].stage) y = voice_run(s, &s->v[0]); }','&& s->v[0].stage) { S1C_PREP_MORPH(); y = voice_run(s, &s->v[0]); } }')
replace('&& s->v[i].stage) y += voice_run(s, &s->v[i]);','&& s->v[i].stage) { S1C_PREP_MORPH(); y += voice_run(s, &s->v[i]); }\n#undef S1C_PREP_MORPH')
pathlib.Path(sys.argv[2]).write_text(s)
```

### s1-render-next/morph-control/voice-check.c

```c
#include "s1c.c"
#include <stdio.h>
#include <assert.h>
int main(void){
 static s1c s;
 static _Alignas(16) uint8_t arena[8192]; static uint16_t wbuf[2205];
 unsigned samples=0,nonzero=0,arp_blocks=0;
 for(unsigned arp=0;arp<2;++arp)for(unsigned ft=0;ft<3;++ft)for(unsigned count=1;count<=6;count+=5){
  float p[150]={0};
  p[ARPON]=(float)arp;p[ARPRATE]=720.f;p[ARPINT]=7.f;p[ARPSTEPS]=4.f;p[BAL]=.21f;p[DET2]=.08f;p[DETM]=1.f;p[A4]=440.f;
  p[VOL1]=.5f;p[VOL2]=.61f;p[SUBV]=.2f;p[NOISEV]=.02f;p[I1]=.33f;p[I2]=.72f;p[BLE]=1.f;
  p[FMIX]=1.f;p[CUT]=2741.f;p[RES]=.26f;p[FENV]=.31f;p[FTYPE]=(float)ft;
  p[FA]=.01f;p[FD]=.22f;p[FS]=.49f;p[FR]=.16f;
  p[AA]=.02f;p[AD]=.3f;p[AS]=.61f;p[AR]=.22f;p[PORT]=.01f;p[MVOL]=.25f;p[BCSR]=44100.f;p[CMG]=1.f;p[CMR]=1.f;p[CRR]=1.f;p[CWR]=1.f;
  p[L1A]=.45f;p[L1R]=1.2f;p[LFENV]=1.f;
  s1c_init(&s,44100.f,p);
  memset(arena,0,sizeof arena); memset(wbuf,0,sizeof wbuf); s1c_attach(&s,arena,wbuf,arena+S1C_RVB_BYTES,S1C_ARENA_BYTES-S1C_RVB_BYTES);
  for(unsigned k=0;k<count;++k)s1c_start(&s,41+5*k,81+k);
  for(unsigned i=0;i<160;++i){
   float l[256],r[256];const unsigned n=19+(i*79)%238;
   if(i==69)for(unsigned k=0;k<count;k++)s1c_stop(&s,41+5*k);
   if(i==105)for(unsigned k=0;k<count;k++)s1c_start(&s,42+5*k,90+k);
   if(i==53)s1c_set(&s,CUT,8111.f);
   if(i==9)s1c_set(&s,I1,1.28f);
   if(i==22)s1c_set(&s,I1,-.125f);
   if(i==50)s1c_set(&s,I2,1.f);
   if(i==91)s1c_set(&s,BLE,0.f);
   if(i==112)s1c_set(&s,MONO,1.f);
   if(i==128)s1c_set(&s,MONO,0.f);
   if(i==129)s1c_start(&s,61,97);
   s1c_render(&s,l,r,n);
#if !S1C_NO_SEQ
   if(arp && s.nlast) ++arp_blocks;
#endif
   for(unsigned k=0;k<n;k++){assert(l[k]==l[k]&&r[k]==r[k]);if(l[k]!=0.f||r[k]!=0.f)nonzero++;}
   fwrite(l,sizeof(float),n,stdout);fwrite(r,sizeof(float),n,stdout);
   fwrite(s.v,sizeof s.v,1,stdout);fwrite(&s.rnd,sizeof s.rnd,1,stdout);samples+=n;
  }
 }
 fprintf(stderr,"%u signed synth frames; %u nonzero; all complete voice/RNG state snapshots\n",samples,nonzero);
 assert(nonzero>samples/2);
#if !S1C_NO_SEQ
 fprintf(stderr,"%u active internal arp blocks\n",arp_blocks); assert(arp_blocks>25);
#else
 (void)arp_blocks;
#endif
}
```

### s1-render-next/morph-native/replay-elite.json

```json
{"worst_us": 23646, "halves": 421, "raw_late": 194, "xruns": 194, "skipped_cycles": 0, "peak": 13142, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 4, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1806, "finished": 1805, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 477, "b": 477}, "joint_audible_spans": 477, "owners": {"count": 4, "owners": [{"ms": 3195, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4138, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5718, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7856, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": false}, "nor_immutable": true, "audio_gate_qualified": false}
```

### s1-render-next/morph-native/replay-pro.json

```json
{"worst_us": 26041, "halves": 411, "raw_late": 163, "xruns": 163, "skipped_cycles": 0, "peak": 13120, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1761, "finished": 1760, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 453, "b": 453}, "joint_audible_spans": 453, "owners": {"count": 5, "owners": [{"ms": 2684, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3183, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4130, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5672, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7821, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```
