# Synth One native morph and scheduling diagnostics

Isolated native PI32v2 Elite and Pro builds from the native-widen-hot source donor.
Both synth engines and original arp remain compiled, INIT-only factory bank policy;
this is an operating resource/timing diagnostic, not all downloadable stock content,
full sound parity, an installable image, or a production candidate.

- Host fast shared-tight morph: 372,736 bit-identical oscillator value/phase probes. All
  906 re-rendered cases have **identical raw reference PCM, decoded compact PCM,
  tuning, event counts, and metrics** versus the frozen tight host baseline
  a084f5ed40a84707147591590b4efe1a8f2edf3cef6ab3d1b33f4d55b7906b19. Same upstream event tape,
  no raw PCM in this recovery. Sound bounded by that existing linear/bitpacked table model.
- Native actual links, data-only section with all codec code in app: XIP over by 1,656/1,644 B.
  Moving exact codec code with the data into the quarantine companion yielded paired
  app margins +280/+292 B; fast KR yields +296/+308 B; POOL_MIN +564 B throughout.
  Paired companion 20,410 B and fast KR 20,372 B, within the 20,480 B replay-only region.
- Fast KR single-gather adjacent residuals and exact power-of-two interpolation avoid
  separate decoding. Cache only the existing pitch/band plan in existing voice words
  at the existing 8-sample frequency update; note phase, live morph position and
  antialias control stay per sample. Strict raw 360MHz leaf on real PI32v2 instructions
  measured cached cycle/tick deltas of -9.1%, -10.2%, -10.8% at 440/880/12000Hz,
  unchanged native phase and output sums. Still 1.98x, 1.80x, 2.99x direct Float32.
- Strict full offset0 loaded 922-key scene, S1 A/arp + Chicago B, original mastering and
  both engines available, zero skips and zero virtual erases/programs: reference hot
  approximation 15,224us/50 XRUN Elite, 14,196us/85 Pro; paired morph 15,190us/132 and
  15,207us/132; fast KR 17,890us/143 and 15,087us/49. All four morph runs emitted 319,488
  PCM frames, had five settled owner readbacks, and failed the requirement.
- Uninstrumented snapshots are endpoint-only state, not a worst-block trace. Paired
  ended with worker mask 0x03 on both; KR Elite ended mask 0, Pro mask 0x13.
  S1 A measured running estimate ~76k timer ticks/256 frames; existing normal planner
  prohibits assigning both S1 and Chicago to the second core (only Miami eligible).
  Separate actual scheduler trial admits only S1, still excludes Chicago and honors stock guards.
  This improved worst loaded time to 12,916us/51 Elite and 13,130us/53 Pro, still failing.
  Worker mask latched 0x04 with observed secondary stack use 1,868 of 4,352 B and
  2,484 B untouched; 2,493/2,483 dispatches, no give-up or bad mailbox. This is neither
  a generalized dual-core memory certification nor adoption for every live program.
- Pre-expanding active rows is not possible with verified persistent free pool: even one
  full folded saw row needs 2,049 int16 values =4,098 B. Quarter triangle one needs 2,050 B;
  all 12 bands across two independent slots can be active during pitch/preset/arp edits.
  Remaining free above the required pool floor is 564 B, and the 9,216-B XPOOL is exactly
  filled by bass B workspace (5,120) and stereo A/B work/output (4,096). The other 5,120-B
  workspace is a scheduled 808-metal/Chicago/Miami union, shared in the stretch; neither
  owner layout nor lifetime permits retaining oscillator hot rows between callbacks.
  A hypothetical tiny clipped cache cannot guarantee a hit across simultaneous arbitrary phase.
- Quarantine placement is **not** production approval: old runtime can write these sectors,
  old installer may not write them, and actual users may have part of an 80KiB custom loop
  there. The tested loaded cue uses no sample-slot writes; prior static offline five-BREAK
  fit proof is separate. A safe production region/negotiated installer migration is still open.

Reproduce inside the existing workspace pins; no private libraries or generated tables are
included. Start with clean matching native-widen-hot tree from parent, copy as isolated
s1-morph-native or s1-morph-native-kr, apply matching subdirectory source patch then copy
.inc/.h alongside s1c.c and run the linker script. Data generator is the prior selection
`s1-morph-compact/encode.py` (same allowed pinned table, `--tol .00005 --share`); fast KR requires
exact single extra zero sentinel (18184→18185 declared bytes) and width ≤10 or raw width=16.
The `results/*-link.txt` and `replay-overlay.py` checks pin the disposable ELF and section SHA.
Never package or install any ELF or diagnostic NOR. Existing restored stock-oracle guard
is still invoked first; overlay additionally verifies exact class/hash/length and erased
virtual receiver. Full 906 host evidence uses `build-host.sh` with private canonical source
dependencies and the existing same-note/tuning replay. No Git or device writes occurred.

Keeping the 808 wholly on core 0 when S1 is a worker regressed both sides:
13,387us/68 Elite and 13,367us/64 Pro; normal split retained. The compact measured-trace
separate binary raises Elite misses 51→63, so strict qualification stays with uninstrumented
figures. Its single observed worst half (12,823us at 5412ms) distributes 10,316us on core 0
(909 6,082 + Chicago 3,515 + other jobs), 8,097us S1 on core 1 and only 16/71us actual
ordered mix waits. Extra 808 prefix outside run_job and tail/master/IRQ overhead not counted.
Do not add the two core sums. No per-original-miss attribution is claimed.

The separate Pro trace (13,238us at 5122ms, 56 late vs 53 uninstrumented) confirms 909
5,969us and Chicago 2,923us local, S1 7,624us worker; core0 ordered waits 1,102us, unlike
Elite. Snapshot records only the observed worst half, not all misses. The 909 Huffman/IMA
span-local rewrite retains exact complete 909 state and all four output buses after 41,600
calls each at O0 and O2, but the strict native result is mixed: 12,810us/48 Elite;
13,149us/64 Pro, zero skips or writes. Don't adopt as a timing fix. The exploratory
parallel last sampler probe did not link (196/184 B overflow even before fail-closed
atomic state publication). It isn't included as a patch. A future offload may use the
5,120 B workspace's trailing region *only* while the selected workspace is reserved by
stage order and after proving timeout-safe publication; unrestricted overlap is unsupported.

## Chicago scope: informative negative, no adoption
The dry oscillator's slave base conversion and unmodulated saturated phase delta are
span-invariant; the parameter path and original rounding, sync resets, XMOD, filters
and sound options were kept. The real C reference guard passed 4,344 complete sample/
state comparisons at -Os and again with ASan/UBSan (LeakSanitizer unavailable in the
sandbox); extra full unsigned increments test F32 boundary/saturation and signed-zero
XMOD. Real PI32 isolated 16,384-frame cases retained exact PCM hashes: held without XMOD
5.50% faster, held XMOD 0.11% faster, moving ramps without XMOD 1.39% faster,
moving ramps with XMOD 0.42% slower. App free XIP dropped 80 B (200/212 B remains);
pool unchanged. Strict loaded results (same no-write/zero-skip runner) remained mixed:
Elite 13,239us/52 late; Pro 12,919us/54 late. Neither clears 10,809us and 0 late.

Separate instrumented worst-half snapshots were Elite 13,294us and Pro 13,096us.
Chicago scoped parts (us): dry/clock/control 1883/2331, wet/master 604/737,
admission 52/49, stereo copy 24/24. Only constituent timing, no claim of matching
uninstrumented phase or all misses. Detached binary duplicate scan gave 384 B gross upper
bound, not legally reclaimable code; four unaligned cold little-endian readers account for
72 B maximum even before calls/relocations. No unsafe 909 offload was linked or run.

## Scheduler split and 909 state-publication bounds
Pricing the existing worker 808 BD/SD prefix in the stage model, with separate measured full versus split 808 work, leaves the original DSP, mixer order, event subdivision and guards untouched. Actual app margins +232/+244 B and +564 B above POOL_MIN. Strict same loaded scene: 12,889us/64 Elite and 13,043us/34 Pro, healthy second core, 4 readbacks, 319,488 PCM frames and zero skips or writes. Against retained worker result 12,916/51 and 13,130/53, mixed, not adopted. Active endpoint mask on both 0x04.

Sound changes when merely coalescing loads: the existing C 909 renderer differs at cut points even with no note change; 256 vs 128+128 changes 256 raw float words from frame128 and final owner state, and five other cuts demonstrate the same. Preserve the 2x256 DMA partition and exact event cuts.

Native unlinked layout: isolated 140-B voice shadow, 4-B completion word and 220 float samples in existing 1,024-B scratch tail after 808 metals, no growth of 5,120-B workspace. Host one-voice prepare+serial commit is bit-identical in 3,200 cases, half discarding shadow, keeping live state untouched and retrying original serial path. Future firmware must consume this in the serial normal voice-mix position before 808/selected workspace's owner reuses the tail. Only the serial owner commits. Missing publication requires existing guarded stop/join before retry/reuse. No proof for >220 frames or shipping live inter-core handoff; the early pre-safety offload exceeded XIP and was never run.
