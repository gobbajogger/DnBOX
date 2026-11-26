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

## 909 post-mix exact-state variants
The ordered 909 output lane computes centered mono before pan, four separate scale
glides, kit sends, and the meter. Fusing arbitrary settled gains was byte-identical
but more than doubled the native leaf time at unity/no sends. Selective fusion retained
the original unity/track fallback and achieved 19.83% faster leaf at attenuation/no
sends or 34.61% with sends, with native raw per-case hashes exact. Its whole strict
results remained mixed: Elite 13,266us/39 late, Pro 13,192us/59.

Current probe demonstrated that all four L/R and voice-send lanes share the same
Float32 moving recurrence in some real terminal snapshots, with settled zero kit
sends. The common variant checks exact bit agreement, preserves each lane's final
gain commit and mixer ordering, and keeps the complete existing fallback on
nonzero kit sends, unequal lanes, unity, or uncertain state. Exact host 20,000
comparisons and ASan/UBSan pass, including both movement and plateaux.
Native 131,072-frame leaf per mode preserves all output fingerprints and is 35.32%
faster for common settled and 49.59% for common moving (about 86us per active
512-frame half at the measured operator rate; not a whole-block timing result).
Both untouched fallbacks add <=0.35% in the isolated leaf.

Unchanged companion and protected ranges. The cold duplicate-only measured savings
are exactly 64 B app on both boards (344/356 B free vs 280/292 B). This simply uses one
identical unaligned little-endian decoder for four existing parser roles, with identical
caller validations and no lift to any region size/reserve. It was not a timing fix:
whole strict cold-only 13,210us/57 Elite, 13,000us/66 Pro. The common 909 arm needs
336 B of app relative to this cold-only link, leaving 8 B Elite, 20 B Pro. Pool remains
564 B above mandated 8,192 B. No production migration approval is implied.

Whole strict common-gain result: Elite 12,888us / 57 late, Pro 12,991us / 40 late.
Both terminal snapshots satisfy the fast-route guard, 4 settled owner readbacks and
319,488 PCM frames each. Zero skipped cycles, virtual erases or writes. The exact
mix pass improves vs cold-reader-only test but not the original baseline on every
metric (Elite +6 late, Pro -13). Release target is still unfulfilled; keep diagnostic.

## 909 native instruction-fetch A/B and release capacity ledger
On same loaded playback and scheduler/observer source, linked the exact same
730-byte d9_render_smp leaf into either APP XIP or the existing boot-copied RAM
code region. Only absolute data load relocations differ in the function (14 byte
positions across seven sites). One identical volatile absolute-call mechanism in
both modes is needed by the native far-call range; this is a controlled diagnostic
pair, not timings of unchanged production baseline. FDATA payload (including
sample streams and LUT), morph, font payload, FITTAB payload are bit-identical;
some other relocated metadata bytes naturally reference different APP addresses
and all original build-specific guards remain checked. No guard window, pool
reserve, FLASH location, or executable lifetime enlarged.

RAM mode consumes 728 B net more RAMTEXT because of final alignment, leaving 1,036
B of already designated RAM code (BLE unaffected). Both physical builds +404/+416
B XIP in the flash diagnostic and +412/+424 B in RAM, with +448 B above required pool
reserve (instrumented). Real host state regression: 41,600 complete 909 state and
four-bus PCM comparisons byte-identical.

360 MHz loaded diagnostic timing (same synthetic S1/ARP/Chicago, same 922 actions,
same confirmed active plans):
- Elite XIP 12,965us / 54 late; RAM 12,881us / 61.
- Pro XIP 13,275us / 64 late; RAM 13,321us / 59.
Each zero skipped cycles and emulator NOR byte-for-byte unchanged; 319,488 PCM
frames, 4 readbacks. All fail. Those peaks have different within-scene phases,
with all-matching worker 0x04 and three stretches but 909 cost still roughly
6.1–6.3ms. Do not interpret the mixed gains as calibrated hardware flash contention
or treat the emulation's per-core cache model as a hardware bus proof. A hardware
contention diagnosis would require matched workload snapshots and real bus counters.
This run does bound practical value of the tested fetch-placement choice and does
not support further flash relocation as a deadline fix. Reasonable next redesign:
transactional dry 909 voice prediction/commit in the proved bounded-220-frame
physical workspace lifetime once actual APP code is reclaimed. Independent untraced
noinline-only links instead spent 28 B, so do not reuse the smaller traced link as a
production free-space claim. Preserve ordered mix and rollback, not unlimited spin.

The production companion and wet-memory inventory uses retained uninstrumented
native maps: no currently approved immutable region for the 20,372 B morph asset;
only 48 B behind current other companion payloads. Proposed slot1 tail would shrink
80KiB to60KiB, even though all stock five samples remained in an offline trial; do not
assume the writable tail empty or reuse it. Exact source 44.1k Float32 RevSC needs
98,760 mutable bytes per owner before owner state. At a representative 0.2162s, the
four original delay rings alone require 228,832 B even bounded to current delay,
or 672,824 B combined with RevSC and Widen for two simultaneous owners. At 2.4s
the same two-owner sum is 5,295,480 B before additional control state and padding.
Current 564 B above protected 8192 B pool or hypothetical replacement total
9,068 B (both 4,252 B wet arenas plus usable slack; required width and reserve held)
does not fit. No mutable flash state, protected region storage or on-device actions.
