# SuperJ6 Delay: match the original Sync grid and 3-second limit

Current authorized ARM64 SHA `6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850`. The actual app callback at .0f20b0 reads the Sync mode, rounds 17×normalized to the detent, and passes (240/BPM) times the selected fraction of a whole note. Exact 18 values reached are (2,3,4,6,8,12,16,24,32,48,64,96,128,256,384,512,768,1024)/128. Row zero is 1/64 of a whole note, **not** Free. Sync off alone uses normalized×1.5 seconds. The current AU constructor at .363e10/.363e20 installs a 3-second maximum and current setter .363f38/.363f50 clamps to it before the parameter ramp; old Build9 independently does the same.

Two exact behavior corrections, no persisted or selectable controls changed. At 120 BPM earliest four Sync words now all give 31.25ms; formerly 1 sample/~11.8/23.6/35.4ms. 34 of the 128 Sync words previously requested more than 3 seconds at 120 BPM and now obey the original clamp; top formerly produced a 16-second request. The existing minimum 1 frame at zero Free time is kept, not claimed as original zero-time parity.

The isolated source-linked C proof covers 4,608 clock calculations across 128 values, free/sync, 9 tempos, 44.1/48 kHz. Every Free arm stayed bit-exact; maximum Sync calculation deviation from simple double formula was 0.0078125 frame, with correct endpoint and clamp. Native paired link on joined diagnostic: Elite and Pro have 524/536 B APP free (+16 B code vs joined baseline), same 3,236 B usable audio pool. No runtime replay or full signed Delay parity claim on these links. Aliasing and live feedback/clock behavior still open.

Apply to the raw include with the retime patch already present; build same host probe old/new with `-ffp-contract=off` as documented in the live-retime receipt. Run verify.py next to the resulting old.so/new.so. Probe below uses that receipt's probe.c and adds clock_frames().

### clock.patch

```diff
--- a/firmware/src/dsp/juno6_delay_native.inc
+++ b/firmware/src/dsp/juno6_delay_native.inc
@@ -8,11 +8,14 @@
 static float native_delay_frames(const juno6_t *s,unsigned raw,float bpm,unsigned sync)
 {
     static const uint16_t grid128[18]={
-        0,3,4,6,8,12,16,24,32,48,64,96,128,256,384,512,768,1024
+        2,3,4,6,8,12,16,24,32,48,64,96,128,256,384,512,768,1024
     }; /* fractions of a whole note /128, same original 18 detents */
     if (raw>127u || sync>1u || !(bpm>0.f) || bpm>10000.f) return 0.f;
     unsigned i=sync?(raw*17u+63u)/127u:0u;
-    float sec=i ? (float)grid128[i]*(1.875f/bpm) : (1.5f/127.f)*(float)raw;
+    float sec=sync ? (float)grid128[i]*(1.875f/bpm) : (1.5f/127.f)*(float)raw;
+    /* Original wrapper clamps seconds to [0,3] before the delay ramp.
+     * Keep the stored grid detent and controls unchanged. */
+    if(sec>3.f)sec=3.f;
     float frames=sec*s->sample_rate;
     if (!(frames<=(SJ6_NATIVE_DELAY_CAP-2u)*SJ6_NATIVE_DELAY_MAX_STRIDE)) return 0.f;
     return frames<1.f ? 1.f : frames;
```

### verify.py

```python
import ctypes as C, json, math, numpy as np
from pathlib import Path
p=Path(__file__).parent
a,b=[C.CDLL(str(p/(k+'.so'))).clock_frames for k in ('old','new')]
for f in (a,b):
 f.argtypes=[C.c_uint,C.c_uint,C.c_float,C.c_float]
 f.restype=C.c_float
# Reached app grid in units of whole notes, seconds then parameter bound.
grid=(1/64,3/128,1/32,3/64,1/16,3/32,1/8,3/16,1/4,3/8,1/2,3/4,1,2,3,4,6,8)
assert len(grid)==18
num=diff=0;max_d=0
for bpm in (20,40,57.25,90,120,127,185.25,260,400):
 for sr in (44100,48000):
  for sync in (0,1):
   for raw in range(128):
    old,new=a(raw,sync,bpm,sr),b(raw,sync,bpm,sr)
    if sync:
     i=math.floor((17*raw/127)+.5)
     exp=max(1.,sr*min(3.,(240/float(bpm))*grid[i]))
     max_d=max(max_d,abs(new-exp))
     assert abs(new-exp)<.03,(raw,bpm,sr,new,exp)
    else: assert np.float32(old).tobytes()==np.float32(new).tobytes()
    num+=1;diff+=old!=new
print(json.dumps(dict(cases=num,changed=diff,max_sync_error_frames=max_d,
 low_cell_120=[(raw,a(raw,1,120,44100),b(raw,1,120,44100)) for raw in range(4)],
 capped_120=sum(b(raw,1,120,44100)==132300 and a(raw,1,120,44100)>132300 for raw in range(128)),
 longest_cell_120=[a(127,1,120,44100),b(127,1,120,44100)]),indent=2))
```

### result.json

```json
{
  "cases": 4608,
  "changed": 716,
  "max_sync_error_frames": 0.0078125,
  "low_cell_120": [
    [
      0,
      1.0,
      1378.125
    ],
    [
      1,
      520.8661499023438,
      1378.125
    ],
    [
      2,
      1041.7322998046875,
      1378.125
    ],
    [
      3,
      1562.598388671875,
      1378.125
    ]
  ],
  "capped_120": 34,
  "longest_cell_120": [
    705600.0,
    132300.0
  ]
}
```

```c
float clock_frames(unsigned raw,unsigned sync,float bpm,float sr){
    juno6_t s={.sample_rate=sr,.typed=1};
    return native_delay_frames(&s,raw,bpm,sync);
}
```

```text
guarded elite XIP 552148 free 524 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000001000000d8ef00007def59efde3db94ff23f165e4fe84840 ELF 183ed0ff32b704462293578f3abf4e24828820b351ca8080dd8a7c8f6fd9069e
guarded pro XIP 552136 free 536 pool usable 3236 read-only FDATA 61432 diagnostic slot3 span 32764 slot3 free 4 read-only FITTAB 4072 header 443952440200000002000000d8ef00007def59ef1d8f9406242ec538f7a42176 ELF 84d281ba38c52c09302f46cdf140574d577ee7ecf41f733f91a8e224ed8dcede
```
