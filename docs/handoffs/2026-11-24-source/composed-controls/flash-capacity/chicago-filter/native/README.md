# Native FAST32 results and next architectural steps
 
Source-only receipt of an isolated all-controls diagnostic. No release or slot
migration. The branch's adjacent host proof generated the 32,770 exact samples.
 
| Board | APP headroom | Above RAM reserve | Loaded strict render | Dropouts |
|---|---:|---:|---:|---:|
| Elite | 416 B | 464 B | 12,402 us | 3 |
| Pro | 424 B | 464 B | 12,323 us | 3 |
 
Matched diagnostic baseline, same feature flags and event sequence: Elite
12,077 us / 3; Pro 12,374 us / 3. Both candidate boots kept audio active,
rendered 620 halves, and left safe/firmware guard flags clear. The two
power-on companion packages are emulator-only. Against this matched base APP
gained 168 B on each, Slot3 moved from 32,764 to 27,780 B, and FITTAB moved
from 4,079 to 3,053 B. FDATA is unchanged at 61,434 B. These are byte
savings, not an accepted slot release or an improvement to worst-block timing.
 
Exact candidate ELFs: Elite `9e0b0a29a640e7499a8437d5149b0e7b4cf6c83a604e6dab1e27308b757a7aa5`
and Pro `ac178c2508a607a30f93fc2dead920ca7742bce02d8ab95a14850ab6be9c0df1`.
 
Rebuild with the authorized paired ramp-fixedpoints donor, using the included
wrapper to emit the private table inputs in scratch only. `rebuild.sh` expects
the original workspace-relative dependencies; when restoring, set `base`,
`root` and the path to the adjacent `filter_words.py` to match the recovered
toolchain and donor. Never put generated p_a.h, p_e.h or the ELFs in version
control. Native matches must not be published without matching release gates.
 
## Architecture constraints to test
 
- Current `c1_allowed()` keeps Chicago off the second core citing a 2,816 B
  stack and a 2,988 B historical watermark, but the current assembly and ELF
  allocate 4,352 B of user stack. The non-BLE guard excludes the bottom 256 B;
  BLE's supervisor and guard setup differs, and does not itself grant stack
  space. Verify runtime stack use and correct event/PCM output before
  expanding eligibility. Do not run two Chicago parts concurrently until the
  shared mutable table cache is removed or made safe. Existing configuration
  forbids Chicago on Core1; concurrency is not established in production.
- Existing shared main-loop scratch is 4 KiB, borrowed by OTA, project LZ,
  PERF and editor frame parsing. The audio interrupt never uses it. A preset
  loader can only borrow an explicitly non-overlapping lease or use bounded
  private scratch. It must never keep this lease past a call with a conflicting
  owner. The 36,420 B project stage is already the live undo baseline, not
  generally spare.
- Ordinary preset browsing can read immutable plain-XIP data outside render,
  validate one bounded record ahead of use and publish a complete state at a
  block boundary. Do not put directory scans or decompression into an audio
  callback. Existing flash erase explicitly silences audio; bank imports and
  deletes must be maintenance operations, not scheduled during playing. A
  storage media calculation must separately include directories, copy-on-write
  reserve, and current live/saved BREAK references.
