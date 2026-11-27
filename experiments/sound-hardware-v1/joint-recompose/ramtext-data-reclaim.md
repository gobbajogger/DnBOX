# Move CPU-only state within the assigned app SRAM

Scoped default-off native diagnostic. This does **not** use the unassigned RAM gap above the Bluetooth host, nor emulator-only memory. Reserve an aligned NOLOAD section after the existing Core1 code in app-owned RAMTEXT, bounded at 0x01C01800. Keep _rt_end unchanged (0x01C0111C), so executable copy loops and the app PC guard do not include this data. The code sink and BLE range are unchanged. Clear only the new section at startup when the diagnostic is enabled. Move existing CPU-only page cache and calibration arrays, not DMA/SPI buffers. No UI layout or calibration value behavior changed.

Both native links: **+1,244 B usable audio pool**, 3,236→4,480 B beyond the 8 KiB base reserve; **−32 B APP headroom** versus the table-dedup donor, Elite/Pro 624/636 B remaining. The three object sizes are unchanged (712+370+160 B, rounded to 1,244). Under strict cold boot, all three relocated data arrays match the baseline byte for byte on both targets. Proof end 0x01C015F8, before controller RAM. No physical memory probe or writes.

Strict paired loaded scene remains unresolved. Elite 23,705 µs/194 XRUNs, Pro 25,862 µs/165 XRUNs. Both actually coaudible, all five owner readbacks, no skipped cycles or bad mailboxes, virtual NOR unchanged. These timing values do not imply performance improvement or memory-layout risk under untested custom projects. Do not enable the option or promote to product before the actual gate. 1,244 B does not cover the audio-history gap.


### move-data.patch

```diff
--- a/firmware/app.ld
+++ b/firmware/app.ld
@@ -41,6 +41,11 @@
     .c1_text : { _c1_start = .; KEEP(*(.c1_text*)) . = ALIGN(4); _c1_end = .; } > RAMTEXT AT > XIP
     _c1_load = LOADADDR(.c1_text);
     _rt_end = _c1_end;                  /* the guard's RAM code window covers both */
+    .ram_spare (NOLOAD) : {
+      . = ALIGN(4); _ram_spare_start = .; *(.ram_spare*) . = ALIGN(4); _ram_spare_end = .;
+    } > RAMTEXT
+    ASSERT(_ram_spare_start >= _c1_end && _ram_spare_end <= ORIGIN(RAMTEXT) + LENGTH(RAMTEXT),
+           "RAM spare data must stay outside code and Bluetooth regions")
     .data : { . = ALIGN(4); _data_start = .; KEEP(*(.bss.fm1_c1_mb)) *(.data*) *(.sdata*) . = ALIGN(4); _data_end = .; } > RAM AT > XIP
     _data_load = LOADADDR(.data);
     .c1stack (NOLOAD) : { *(.bss.c1stack) } > C1STK
--- a/firmware/src/app/main_fm1.c
+++ b/firmware/src/app/main_fm1.c
@@ -4,6 +4,9 @@
 /* X0X on the FM-1: boot (WDT first, boot-loop guard, guards), LCD, input (TIMER5 10 kHz),
  * audio (ALNK0), USB; then the main loop: UI at ~60 frames/s, input polled in between. */
 extern uint32_t _data_start[], _data_end[], _data_load[], _bss_start[], _bss_end[];
+#if DNBOX_LAB_RAM_SPARE
+extern uint32_t _ram_spare_start[], _ram_spare_end[];
+#endif
 extern uint32_t _xpool_start[], _xpool_end[];
 extern uint32_t _pool_start[], _pool_end[], _rt_start[], _rt_end[], _rt_load[], _c1_start[], _c1_end[], _c1_load[];
 
@@ -2590,6 +2593,10 @@
     irq_stack_mark();                               /* before any interrupt: PERF's IRQ STACK */
     for (d = _bss_start; d < _bss_end; d++)
         *d = 0;
+#if DNBOX_LAB_RAM_SPARE
+    for (d = _ram_spare_start; d < _ram_spare_end; d++)
+        *d = 0;
+#endif
     for (d = _pool_start; d < _pool_end; d++)
         *d = 0;
     for (d = _xpool_start; d < _xpool_end; d++)
--- a/firmware/src/app/ui.c
+++ b/firmware/src/app/ui.c
@@ -2974,7 +2974,11 @@
 #endif
 static const uint8_t title_chicago[]={PT_OSCILLATOR,PT_FILTER_1,PT_FILTER_2,PT_ENVELOPE,PT_AMPLIFIER,PT_ARPEGGIATOR,PT_EFFECT};
 static const uint8_t title_miami[]={PT_CARRIER,PT_XMOD,PT_FILTER,PT_WOBBLE,PT_OUTPUT};
+#if DNBOX_LAB_RAM_SPARE
+static pages_t pg __attribute__((section(".ram_spare")));
+#else
 static pages_t pg X0X_POOL;
+#endif
 typedef struct { pref_t r[4]; char title[12]; int page, n; } page_query_t;
 /* The descriptor emitter has one explicit output contract. Query mode records
  * one requested page plus the count, without a second persistent page table. */
--- a/firmware/hal/smk_input.h
+++ b/firmware/hal/smk_input.h
@@ -125,7 +125,12 @@
 #ifndef SMK_BIG
 #define SMK_BIG                   /* (a section for big buffers, e.g. a zero-initialised RAM pool: x0x.c) */
 #endif
+#if DNBOX_LAB_RAM_SPARE
+static uint16_t smk_trim_pad[SMK_NPAD][5] __attribute__((section(".ram_spare")));
+static uint16_t smk_trim_key[FM1_NKEY][5] __attribute__((section(".ram_spare")));
+#else
 static uint16_t smk_trim_pad[SMK_NPAD][5] SMK_BIG, smk_trim_key[FM1_NKEY][5] SMK_BIG;
+#endif
 
 /* ------------------------------------------------------------------ state --- */
 static volatile struct {
```

### check-relocation.py

```python
from pathlib import Path
import re,subprocess,hashlib,json
base=Path('sj6-parallel-joined')
v=Path(__file__).parent
def read(d,model,n):
    elf=d/'tree'/'joint-diagnostic-later'/model/'never-install.elf'
    l=subprocess.check_output(['readelf','-sW',elf],text=True)
    p=re.search(r'^\s*\d+:\s+([a-f0-9]+)\s+(\d+) OBJECT\s+LOCAL\s+DEFAULT\s+\d+ '+n+'$',l,re.M)
    assert p is not None,(elf,n)
    start,size=int(p[1],16),int(p[2])
    ram=Path((d/'cold.log').read_text().splitlines()[0])/f'{model}-offset--1'/'ram.bin'
    b=ram.read_bytes()
    assert len(b)==1048576
    return start,size,b[start-0x1c00000:start-0x1c00000+size]
report={}
for m in ('elite','pro'):
 report[m]={}
 for n in ('pg','smk_trim_key','smk_trim_pad'):
    o,sz,a=read(base,m,n);p,t,z=read(v,m,n)
    assert 0x1c20000<=o<0x1c74000 and 0x1c0111c<=p<p+t<=0x1c015f8 and sz==t
    if n!='pg':assert a==z and any(z),(m,n)
    report[m][n]=dict(old=hex(o),new=hex(p),bytes=t,cold_bytes_equal=a==z,
                         nonzero_bytes=sum(x!=0 for x in z),new_sha=hashlib.sha256(z).hexdigest())
print(json.dumps(report,indent=2))
```

### section-proof.txt

```text
  [ 7] .ram_text         PROGBITS        01c00400 086300 000abc 00  AX  0   0  2
  [ 8] .c1_text          PROGBITS        01c00ebc 086dbc 000260 00  AX  0   0  2
  [ 9] .ram_spare        NOBITS          01c0111c 08701c 0004dc 00  WA  0   0  4
  [13] .pool             NOBITS          01c20000 000500 050e80 00  WA  0   0  4
01c015f8 B _ram_spare_end
01c0111c B _ram_spare_start
01c0111c T _rt_end
01c00400 T _rt_start
01c0111c 000002c8 b pg
01c013e4 00000172 b smk_trim_key
01c01556 000000a0 b smk_trim_pad
  [ 7] .ram_text         PROGBITS        01c00400 086300 000abc 00  AX  0   0  2
  [ 8] .c1_text          PROGBITS        01c00ebc 086dbc 000260 00  AX  0   0  2
  [ 9] .ram_spare        NOBITS          01c0111c 08701c 0004dc 00  WA  0   0  4
  [13] .pool             NOBITS          01c20000 000500 050e80 00  WA  0   0  4
01c015f8 B _ram_spare_end
01c0111c B _ram_spare_start
01c0111c T _rt_end
01c00400 T _rt_start
01c0111c 000002c8 b pg
01c013e4 00000172 b smk_trim_key
01c01556 000000a0 b smk_trim_pad
```

### relocation-report.json

```json
{
  "elite": {
    "pg": {
      "old": "0x1c39208",
      "new": "0x1c0111c",
      "bytes": 712,
      "cold_bytes_equal": true,
      "nonzero_bytes": 5,
      "new_sha": "b223f5d35ba69c32ca4ea939281ef7956cf2ef36ee954ad12e4c2146bbb7e228"
    },
    "smk_trim_key": {
      "old": "0x1c3a914",
      "new": "0x1c013e4",
      "bytes": 370,
      "cold_bytes_equal": true,
      "nonzero_bytes": 333,
      "new_sha": "de912a16226abcb3eb4c445c4a313d63b0198e651b3a98f55eac3272cf5fbd7a"
    },
    "smk_trim_pad": {
      "old": "0x1c3aa86",
      "new": "0x1c01556",
      "bytes": 160,
      "cold_bytes_equal": true,
      "nonzero_bytes": 144,
      "new_sha": "100e694c17507c742162caf55686bfd87fed34036780d8501e6cb9cc73a16ba8"
    }
  },
  "pro": {
    "pg": {
      "old": "0x1c39208",
      "new": "0x1c0111c",
      "bytes": 712,
      "cold_bytes_equal": true,
      "nonzero_bytes": 5,
      "new_sha": "b223f5d35ba69c32ca4ea939281ef7956cf2ef36ee954ad12e4c2146bbb7e228"
    },
    "smk_trim_key": {
      "old": "0x1c3a914",
      "new": "0x1c013e4",
      "bytes": 370,
      "cold_bytes_equal": true,
      "nonzero_bytes": 333,
      "new_sha": "de912a16226abcb3eb4c445c4a313d63b0198e651b3a98f55eac3272cf5fbd7a"
    },
    "smk_trim_pad": {
      "old": "0x1c3aa86",
      "new": "0x1c01556",
      "bytes": 160,
      "cold_bytes_equal": true,
      "nonzero_bytes": 144,
      "new_sha": "100e694c17507c742162caf55686bfd87fed34036780d8501e6cb9cc73a16ba8"
    }
  }
}
```

### replay-elite.json

```json
{"worst_us": 23705, "halves": 421, "raw_late": 194, "xruns": 194, "skipped_cycles": 0, "peak": 13142, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1806, "finished": 1805, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 477, "b": 477}, "joint_audible_spans": 477, "owners": {"count": 5, "owners": [{"ms": 2661, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3241, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4118, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5717, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7832, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```

### replay-pro.json

```json
{"worst_us": 25862, "halves": 412, "raw_late": 165, "xruns": 165, "skipped_cycles": 0, "peak": 13151, "frames": 319488, "sealed_boot": true, "settled_owner_readbacks": 5, "meets_timing": false, "core1": {"enabled": 1, "gave_up": 0, "alive": 1, "dispatched": 1763, "finished": 1762, "bad_mailbox": 0}, "accepted": false, "cue": 1, "individually_audible_spans": {"a": 454, "b": 454}, "joint_audible_spans": 454, "owners": {"count": 5, "owners": [{"ms": 2705, "seq": 10, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 3253, "seq": 11, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 4130, "seq": 12, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 5671, "seq": 13, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}, {"ms": 7810, "seq": 14, "bank": 2, "status": 1, "cutoff": 115, "bankB": 3, "statusB": 1, "arp": 1}], "allScheduledOwnerReadbacksAgree": true}, "nor_immutable": true, "audio_gate_qualified": false}
```
