# Synth One LP recurrence: exact local speed-up, not a heavy-scene fix

The LP filter already runs the same four stages twice. Keeping those intermediate values in locals avoids the intermediate five-word write/reload, while preserving Float32 ordering including the initial positive zero. BP and HP unchanged. Host differential: 4,194,304 recurrent samples and all five state words match bit for bit. Whole compact host core: 144,168 nonzero signed stereo frames and voice/RNG state exact across LP/BP/HP, single/multiple voices, edits, mixed partitions, release and retrigger.

Native isolated paired kernel (Pi32, clock 360 MHz, common 24 MHz timer): 32,768 recurrent inputs, zero output/state mismatches, 369,169→312,585 ticks (15.3% including timer overhead). This is per isolated kernel, not a prediction of total CPU improvement. Leaf +48 XIP B, identical pool.

## Same combined native cue, fresh baseline and one change

| Model | Diagnostic | Free XIP | Pool after reserve | Peak half | XRUNs | Both audible spans |
|---|---|---:|---:|---:|---:|---:|
| Elite | Baseline | 788 B | 2588 B | 25,851 µs | 169 | 455 |
| Elite | LP scalar | 740 B | 2588 B | 26,870 µs | 168 | 456 |
| Pro | Baseline | 800 B | 2588 B | 23,286 µs | 198 | 407 |
| Pro | LP scalar | 752 B | 2588 B | 22,983 µs | 199 | 485 |

Native replay: both engines proved audible, five typed-owner readbacks each, no skipped cycles or bad mailboxes, NOR invariant. The candidate shows **no useful board-level dropout improvement**. Do not treat it as a gate or combine blindly. Existing smaller FX, not the full matched effects path. No real hardware, no release.

Recompose both from c24f2f2f as documented; change only this patch for the candidate. `run-joint.sh` below regenerates baseline and candidate from existing built trees without any native device. See `stage-native.sh`, `bench.c`, `start.S`, and `bench.ld` (local lab artifacts) for the isolated raw native test; 600 ms at `--bin bare-padded.bin --entry 0x02000120 --mhz 360 --shot 600`; read four little-endian 32-bit words at dump offset 0x10000: magic 0x4c503131, baseline ticks, candidate ticks, mismatch count.


### ladder.patch

```diff
--- a/firmware/src/dsp/s1c/s1u.c
+++ b/firmware/src/dsp/s1c/s1u.c
@@ -94,15 +94,19 @@
 }
 float s1u_ladder(float *h, float in, float A, float fb)
 {
-    float o = 0.0f;
-    for (int k = 0; k < 2; k++) {
-        float tmp = h[3], v = in - fb * 0.5f * (h[3] + h[4]);
-        float z1 = v + A * (h[0] - v), z2 = z1 + A * (h[1] - z1), z3 = z2 + A * (h[2] - z2), z4 = z3 + A * (h[3] - z3);
-        h[0] = z1; h[1] = z2; h[2] = z3; h[3] = z4; h[4] = tmp;
-        o += z4;
-    }
-    return 0.5f * o;
+    const float h0=h[0],h1=h[1],h2=h[2],h3=h[3],h4=h[4];
+    const float r=fb*0.5f;
+    const float v=in-r*(h3+h4);
+    const float z1=v+A*(h0-v), z2=z1+A*(h1-z1);
+    const float z3=z2+A*(h2-z2), z4=z3+A*(h3-z3);
+    const float v2=in-r*(z4+h3);
+    const float y1=v2+A*(z1-v2), y2=y1+A*(z2-y1);
+    const float y3=y2+A*(z3-y2), y4=y3+A*(z4-y3);
+    h[0]=y1;h[1]=y2;h[2]=y3;h[3]=y4;h[4]=z4;
+    /* Keep initial +0; matters for signed zero. No FMA contraction. */
+    return 0.5f*((0.0f+z4)+y4);
 }
+
 float s1u_hp2(float *z, float x, float a1, float a4, float a5)
 {
     float t = x - a4 * z[0] - a5 * z[1], o = (t - 2.0f * z[0] + z[1]) * a1;
```

### run-joint.sh

```bash
#!/usr/bin/env bash
set -euo pipefail
R=$(cd "$(dirname "$0")/.." && pwd)
S="$R/s1-organ-wet-recovery/native-ed-brk-recompose"
H="$R/restored-harness-inspect/helpers"
O="$R/remote-typed-load"
for d in "$S" "$R/s1-render-next/integrated"; do
  e="$d/tree/joint-diagnostic-later/elite/never-install.elf"
  p="$d/tree/joint-diagnostic-later/pro/never-install.elf"
  if [[ "$d" != "$S" ]]; then cp "$S"/{owner-scan.mjs,cue.py,run-actual.sh} "$d/"; fi
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

### check.c

```c
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
extern float s1u_ladder(float*,float,float,float), next_ladder(float*,float,float,float);
static uint32_t r=1;
static uint32_t rng(void){return r=r*1664525u+1013904223u;}
static float wave(void) {uint32_t n=rng();if((n&7)==0)return (n&8)?-0.f:0.f;return (float)(int16_t)(n>>16)*0x1p-15f;}
int main(void){
 unsigned cases=0;
 for(unsigned h=0;h<4096;h++){
    float x[5],y[5];
    for(unsigned k=0;k<5;k++) x[k] = wave();
    memcpy(y,x,sizeof x);
    for(unsigned i=0;i<1024;i++){
      float input=wave(), a=(float)(rng()&65535)*0x1p-16f;
      float f=(float)(rng()&65535)*0x1p-15f;
      float p=s1u_ladder(x,input,a,f),q=next_ladder(y,input,a,f);
      assert(!memcmp(x,y,sizeof x)&&!memcmp(&p,&q,sizeof p)); ++cases;
    }
 }
 printf("%u exact recurrent stereo-independent voice LP sample/state transitions\n",cases);
}
```

### voice-check.c

```c
#include "s1c.c"
#include <stdio.h>
#include <assert.h>
int main(void){
 static s1c s;
 static _Alignas(16) uint8_t arena[8192]; static uint16_t wbuf[2205];
 unsigned samples=0,nonzero=0;
 for(unsigned ft=0;ft<3;++ft)for(unsigned count=1;count<=6;count+=5){
  float p[150]={0};
  p[BAL]=.21f;p[DET2]=.08f;p[DETM]=1.f;p[A4]=440.f;
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
   s1c_render(&s,l,r,n);
   for(unsigned k=0;k<n;k++){assert(l[k]==l[k]&&r[k]==r[k]);if(l[k]!=0.f||r[k]!=0.f)nonzero++;}
   fwrite(l,sizeof(float),n,stdout);fwrite(r,sizeof(float),n,stdout);
   fwrite(s.v,sizeof s.v,1,stdout);fwrite(&s.rnd,sizeof s.rnd,1,stdout);samples+=n;
  }
 }
 fprintf(stderr,"%u signed synth frames; %u nonzero; all complete voice/RNG state snapshots\n",samples,nonzero);
 assert(nonzero>samples/2);
}
```

### bench.c

```c
#include <stdint.h>
#include "fm1_time.h"
extern float s1u_ladder(float*,float,float,float), next_ladder(float*,float,float,float);
static uint32_t r=3;
static uint32_t rnd(void){return r=r*1664525u+1013904223u;}
static unsigned eq(float p,float q){union {float f;uint32_t u;} a={.f=p},b={.f=q};return a.u==b.u;}
void bench_start(void){
 volatile uint32_t *o=(void*)0x01c10000;
 fm1_time_init();o[0]=0;
 uint32_t bad=0,now[2]={0,0};
 float a[5]={0},b[5]={0};
 for(unsigned i=0;i<32768;i++){
   uint32_t q=rnd();float in=(q&3)?(float)(int16_t)(q>>16)*0x1p-15f:(q&4)?-0.f:0.f;
   float g=(float)(rnd()&65535)*0x1p-16f, f=(float)(rnd()&65535)*0x1p-15f;
   uint32_t t=fm1_ticks();float x=s1u_ladder(a,in,g,f);now[0]+=fm1_ticks()-t;
   t=fm1_ticks();float y=next_ladder(b,in,g,f);now[1]+=fm1_ticks()-t;
   if(!eq(x,y))bad++;
   for(unsigned k=0;k<5;k++)if(!eq(a[k],b[k]))bad++;
 }
 o[1]=now[0];o[2]=now[1];o[3]=bad;o[0]=0x4c503131;
}
```

### stage-native.sh

```bash
#!/usr/bin/env bash
set -euo pipefail
P=$(cd "$(dirname "$0")";pwd); R=$(cd "$P/..";pwd)
S="$R/s1-organ-wet-recovery/native-ed-brk-attribution/tree/firmware/src/dsp/s1c"; T="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
python "$P/stage.py" "$S/s1u.c" "$P/candidate.c"
F=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$T/include" -I"$S" -I"$R/fw/firmware/hal" -I"$R/fw/firmware/src")
"$T/bin/clang" "${F[@]}" -c "$S/s1u.c" -o "$P/a.o"
"$T/bin/clang" "${F[@]}" -Ds1u_ladder=next_ladder -c "$P/candidate.c" -o "$P/b.o"
"$T/bin/clang" "${F[@]}" -c "$P/bench.c" -o "$P/c.o"
"$T/bin/clang" "${F[@]}" -c "$P/start.S" -o "$P/s.o"
"$T/bin/ld" --gc-sections --allow-multiple-definition -T "$P/bench.ld" "$P/"{s,c,a,b}.o -o "$P/bare.elf"
"$T/bin/objcopy" -O binary -j .text "$P/bare.elf" "$P/bare.bin"
{ dd if=/dev/zero bs=288 count=1 status=none; cat "$P/bare.bin"; } > "$P/bare-padded.bin"
"$T/bin/nm" -S "$P/bare.elf"|grep -E 's1u_ladder$|next_ladder$'
```

### start.S

```asm
.section .text.crt0,"ax",@progbits
.globl _start
_start:
   cli
   sp = 0x01c18000
   ssp = 0x01c18400
   call bench_start
   goto .
```

### bench.ld

```text
ENTRY(_start)
SECTIONS {
  . = 0x02000120;
  .text : { KEEP(*(.text.crt0)) *(.text*) *(.rodata*) }
  .bss 0x01c04000 (NOLOAD) : { *(.bss*) *(COMMON) }
  /DISCARD/ : { *(.eh_frame*) *(.comment*) }
}
```

### stage.py

```python
from pathlib import Path
from hashlib import sha256
import sys
src,out=map(Path,sys.argv[1:]); raw=src.read_bytes()
assert sha256(raw).hexdigest()=='7fa9df89ede499229f67d267cbfd58914a6a5b3deb41725db96b14d1cab7356e'
s=raw.decode(); a=s.index('float s1u_ladder(float *h,'); b=s.index('\nfloat s1u_hp2(',a)
new='''float s1u_ladder(float *h, float in, float A, float fb)
{
    const float h0=h[0],h1=h[1],h2=h[2],h3=h[3],h4=h[4];
    const float r=fb*0.5f;
    const float v=in-r*(h3+h4);
    const float z1=v+A*(h0-v), z2=z1+A*(h1-z1);
    const float z3=z2+A*(h2-z2), z4=z3+A*(h3-z3);
    const float v2=in-r*(z4+h3);
    const float y1=v2+A*(z1-v2), y2=y1+A*(z2-y1);
    const float y3=y2+A*(z3-y2), y4=y3+A*(z4-y3);
    h[0]=y1;h[1]=y2;h[2]=y3;h[3]=y4;h[4]=z4;
    /* Keep initial +0; matters for signed zero. No FMA contraction. */
    return 0.5f*((0.0f+z4)+y4);
}
'''
out.write_text(s[:a]+new+s[b:])
```

### integrated/replay-elite.json

```json
{"worst_us": 26870, "halves": 413, "raw_late": 168, "xruns": 168, "skipped_cycles": 0, "peak": 13144, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1767, "finished": 1766, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 456, "b": 456}, "joint_audible_spans": 456, "owners": {"count": 5, "owners": [{"ms": 2601, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3241, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4126, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5695, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7813, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### integrated/replay-pro.json

```json
{"worst_us": 22983, "halves": 424, "raw_late": 199, "xruns": 199, "skipped_cycles": 0, "peak": 13140, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1822, "finished": 1821, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 485, "b": 485}, "joint_audible_spans": 485, "owners": {"count": 5, "owners": [{"ms": 2684, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3159, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4120, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5723, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7857, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```
