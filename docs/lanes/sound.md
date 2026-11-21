# Sound lane

## emu360 timing margin

The owner requested a worst block of 10,909 µs or less against the 11,609 µs limit, with zero late and unchanged audio behavior, flash use and 8,192 B pool reserve. Both boards run the stock firmware from `ca0c00a4e5863132de4a66f5d1c477c14d699be5` in `fm1-live` at 360 MHz with `FM1_STRICT=1`. The 8-second scene has 922 events. Each shift moves every press, release and MIDI event by the same amount.

| Scene shift | Current Elite (µs) | Current Pro (µs) | Historical Elite replay (µs) |
| --- | ---: | ---: | ---: |
| +0 ms | 11,402 | 11,289 | 11,406 |
| +1 ms | 11,323 | 11,318 | 11,440 |
| +2 ms | 11,319 | 11,087 | 10,921 |
| +3 ms | 11,282 | 11,241 | 11,438 |
| +4 ms | 11,297 | 11,303 | 11,438 |
| +5 ms | 11,272 | 11,272 | 10,922 |

The worst current shifted run was **11,323 µs Elite** (+1 ms, 414 µs over target) and **11,318 µs Pro** (+1 ms, 409 µs over). Each stock or historical shot completed 648 halves with **0 late during the scene and 1 raw late**. The first IRQ reaches the second DMA half with less than 1 ms left before the next boundary. The separate same-image 2-second warmed boot also records one late. The historical d4088e60 Elite replay reproduced the owner’s 11,440 µs at +1 ms and the previously published 11,406 µs unshifted.

The current Elite unshifted maximum occurred in half 603, at 7.466 s. The 909 and Chicago bass consumed 78,993 and 75,964 guest timer ticks in that half. A diagnostic mixer/planner variant reached 11,291 µs unshifted. It changed the app footprint by −48 B and the first DAC difference occurred at stereo frame 160,324. Its timing does not qualify it for production. The production Elite and Pro images remain 546,516 and 546,508 B. Each retains 8,972 B of pool, or 780 B above reserve. The scene requests BREAK events and the stock image has no optional factory loop bank installed. [Reproduction and measurements](../../experiments/sound-hardware-v1/strict-timing/README.md)

### Native changes under test

| Elite experiment | Unshifted peak (µs) | Occupied application delta | Raw late | Scene late |
| --- | ---: | ---: | ---: | ---: |
| Stock | 11,402 | 0 B | 1 | 0 |
| Planner and settled mixer gain | 11,291 | −48 B | 1 | 0 |
| Mixer and quiet-tail accounting | 11,125 | −240 B | 1 | 0 |
| Planner hysteresis | 11,382 | 0 B | 1 | 0 |
| Chicago finite-check simplification | 11,272 | −96 B | 1 | 0 |
| 909 stereo accumulation, IMA and tom changes | 11,108 | −80 B | 1 | 0 |
| USB initialized before audio DMA | 11,363 | 0 B | 0 | 0 |
| Combined Chicago, 909, mixer and USB ordering | 11,238 | −400 B | 0 | 0 |

The −400 B combination also measured 11,238 µs unshifted on Pro. Initializing USB before DMA lets the first DMA interrupt run on time. Its 8-second captures complete 647 halves because DMA starts later. The stock image completes 648. The scene and stop time stay fixed. A full-size variant specializes the DJ filter, limiter knee and kick path. Fresh native links confirm exactly **546,516 B Elite / 546,508 B Pro** with 8,972 B free pool on both. Each variant has its own ELF, package and factory flash. [Native sizes and differential](../../experiments/sound-hardware-v1/strict-timing/occupied-parity-proof.tar.gz)

The −400 B combination matched all 50 deterministic audio and state outputs across 17 host scenarios. In the unshifted Elite live run, however, five of 36,420 project bytes differed from stock. Two 909 step masks, the 808 pattern length and two sound settings changed. Of the post-PLAY event rows, 296 matched at the same rendered frame, three were present only on stock and one only on the candidate. The phase-sensitive physical setup also yields different project bytes across shifts on unmodified stock. None of these candidates has been enabled. [Control, project, onset and timing receipts](../../experiments/sound-hardware-v1/strict-timing/current-sweep-proof.tar.gz)

### Exact-size emu360 results

Both candidates occupy 546,516 B on Elite and 546,508 B on Pro. Both retain 8,972 B of free pool. The second candidate computes the original 909 nonlinear table coordinate directly from the triangle phase. All 75,497,473 reachable rounded inputs matched word for word. Both candidates passed 17 fixed-input production host scenarios and matched all 50 state and audio files.

| Shift | First exact-size Elite (µs) | First exact-size Pro (µs) | Direct-coordinate Elite (µs) | Direct-coordinate Pro (µs) |
| --- | ---: | ---: | ---: | ---: |
| Control | 11,144 | 11,158 | 10,652 | 10,903 |
| +1 ms | 11,080 | 10,844 | 10,908 | 10,902 |
| +2 ms | 11,148 | 11,142 | 10,877 | 10,813 |
| +3 ms | 11,136 | 11,057 | 10,589 | 10,939 |
| +4 ms | 11,133 | 11,181 | 10,937 | 10,904 |
| +5 ms | 10,931 | 10,827 | Pending | Pending |

All listed candidate shots ended at 8,000 ms with 922 uniformly shifted events, zero skipped emulator cycles and zero raw or scene late. They completed 647 halves because USB settles before audio DMA starts. Stock completed 648. Extending the +1 ms Pro control on the first candidate to 8,012 ms produced 648 halves and the same 10,844 µs maximum. [Per-shot image, project and DAC identities](../../experiments/sound-hardware-v1/strict-timing/candidate-measurements.json)

For the measured +1 through +5 ms direct-coordinate shots, Elite recorded 10,937 µs at +4 ms, 3/4 measured offsets at or below target and Pro recorded 10,939 µs at +3 ms, 3/4 measured offsets at or below target. At +1 ms the same-offset native project differed from stock in 11 bytes on Elite and four on Pro. Both contain a changed 909 hit mask. The candidate remains experimental. [DSP source, native sizes, fixture and differential](../../experiments/sound-hardware-v1/strict-timing/direct-coordinate-proof.tar.gz)

### Playback of one identical saved project

Stock Elite performed a normal Stop and Save to a disposable emulator flash file. Both images subsequently loaded the exact same 36,420-byte project through normal firmware boot. A single Play press replaced all programming gestures for this supplemental comparison. Stock and the first exact-size candidate emitted the same 276 musical and control events at the same rendered frames over 123,392 frames. The DAC was bit-identical over the 207 corresponding halves that both completed by their DMA deadline, covering 105,984 stereo frames.

| Image | Worst block | Raw late | Late during playback test |
| --- | ---: | ---: | ---: |
| Stock Elite | 12,923 µs | 34 | 33 |
| First exact-size Elite candidate | 12,579 µs | 6 | 6 |

The continuous DAC capture differs at missed deadlines. These saved-project results use a different stimulus from the 922-event sweep above. [Save/load, stimulus, event and PCM proof](../../experiments/sound-hardware-v1/strict-timing/saved-project-and-events.tar.gz)

### S6 feedback and storage

A two-cursor experiment with four checkpoints needs 2,712 B per owner and returned 176,400 original wet words exactly with an ideal random-access dry source. The first wet callback still needs 49,662 source frames per owner. An optimistic 360 MHz kernel needed 19.769 ms and 23.582 ms for its two cursors, or 43.351 ms serially for one owner. Even with a core per cursor, the larger cursor exceeds 11.609 ms. The stock app and both 1 MiB flash images contained zero exact nonzero 128-sample source blocks in the tested formats. The live feedback, independent part histories and actual dry-source generation still need a fitting implementation. [Source, model receipts and replay](../../experiments/sound-hardware-v1/synthone-delay/checkpoint-provider-feedback.tar.gz)

Forecasting instead of waiting for the tap brought one owner's predicted wet half to 893 µs. Feeding 378 earlier frames per owner per callback completed the prewarming in time. One legal live delay-time edit at frame 242,550 then changed all 2,048 tap words on that owner. The peer stayed byte-identical. Exact regeneration for the edit took 23.211 ms with an ideal source, including 15.325 ms for the longer cursor alone. S6 remains FAIL. [Live edit and target states](../../experiments/sound-hardware-v1/synthone-delay/prewarm-live-edit-proof.tar.gz)

The separate zero-XMod Chicago optimization matched 840,000 host cases and all 60 target state and PCM cases. It saved 33.9 µs at 256 moving frames with zero XMod, and added 3.1 µs to one nonzero-XMod case. [Target differential](../../experiments/sound-hardware-v1/strict-timing/chicago-xmod-proof.tar.gz)

## Production Synth One INIT and original comparisons, 2026-11-21 UTC

The normal key, cell and render paths now play Synth One INIT on both owners. The former fixed one-sixth gain failed all eight original note/chord scenes. The original sums six voices at unity, and the corrected firmware matches that level. The existing Juno with chorus, Chicago and Miami still build. Older descriptions of a silent slot below refer to their historical checkpoints.

| Item | Status | Current evidence |
| --- | --- | --- |
| S1 | FAIL | INIT plays with independent voices, pitch bend, damper, note reuse and panic. The [compiled production selector and per-program inventory](../../experiments/sound-hardware-v1/factory-admission/README.md) show 0/694 complete original programs selectable. In [2,048 production callback blocks per scene](../../experiments/sound-hardware-v1/stock-dry/callback.md), dual INIT missed 0 CPU and 8 wall deadlines. Adding 808/909 missed 1 CPU and 8 wall deadlines. All samples were finite. Neither scene met the zero-miss contract. |
| S2 | FAIL | The [dry-note comparison](../../experiments/sound-hardware-v1/stock-dry/README.md) passed 180/180 static and 72/72 modulated notes at unity. Eight actual-part comparisons passed including two chords. [Oscillator controls](../../experiments/sound-hardware-v1/band_morph/README.md) passed 18/18 controlled notes. The 694-row inventory records empty metrics for unavailable full-program renders. |
| S3 | FAIL | The [direct AMP2 replay](../../experiments/sound-hardware-v1/sj6-amp2/README.md) proves four additional second-layer controls and owner isolation in the compact candidate. The packed macro drops these original fields. A [separate typed application entry](../../experiments/sound-hardware-v1/sj6-amp2/typed-entry.md) applied all four through the real audio block on both owners. It has no shipping bank importer. A separate pre-note waveform/PWM entry qualifies both layers on 474/640 authored programs, but its native link misses the required XIP reserve. All 64 controls and complete FX routing remain unverified. |
| S4 | BLOCKED | [Original inventory and capture contract](../sound/superj6-ab.md). The available original iOS binary requires a compatible host. The recovered 56 WAVs are drum assets and component comparisons. Zero of 14 complete original preset renders have verified stimuli and matching audio. |
| S5 | FAIL | All 640 SuperJ6 records across 12 banks decoded, with 90,549 source field rows preserved. The compact candidate applied four fields per program, 2,560 writes total. All 2,560 values also matched bit-for-bit after 320 actual block boundaries. The source has 640 distinct complete records and only 539 distinct AMP2 tuples. Full parameter binding and bank selection remain incomplete. |
| S6 | FAIL | The stock INIT image fits both models. Elite has 6,156 bytes free and Pro 6,164, with pool margin 780 on each. [Per-engine RAM/flash](sound-memory.md) and [original delay history counterexamples](../../experiments/sound-hardware-v1/synthone-delay/README.md). Full simultaneous effects and factory instruments have no passing physical image. |
| S7 | PASS | Case-insensitive whole-word scan of firmware, tests and web has zero matches. |
| C1 | OPEN | The canonical Local gate is running on source ca0c00a4e5863132de4a66f5d1c477c14d699be5. The full suite has recorded failures. Four focused reference reruns passed after restoring seven pinned source objects. The TouchOSC test passed with the verified upstream Lua 5.1 interpreter. The original key-sound test and its missing-cue control passed. The drum AMP test passed 238 normal checks, both graph controls and both caption mutants. Five browser safety checks passed. The full browser test still requires the pinned headless shell 1234, whose download returned HTTP 403. LeakSanitizer also recorded a ptrace permission failure in this environment. The terminal gate log is pending. Focused INIT, slot, legacy and Juno tests pass. All four original comparison checkouts are available at their exact pinned commits. The interrupted pre-change suite already recorded host failures, including LeakSanitizer terminating in multiple host binaries. |
| C2 | PASS | Consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` is an ancestor. No deleted paths in the no-rename comparison. |
| C3 | OPEN | No screen layout source changed. The designated independent release review has not signed off. |
| C4 | OPEN | New evidence consists of source, measurements and reproduction material. The exact-head gate owns the repository-wide limits receipt. |
| C5, C6 | FAIL | Checklist remains open. PR #973 stays in draft pending qualification and integration review. |

The [whole-app legacy controls](../../experiments/sound-hardware-v1/sj6-amp2/entry/README.md) include the failing compact-backend legacy test build transcripts. The [focused stock runtime log](../../experiments/sound-hardware-v1/stock-dry/freeze-test.log) and isolated oscillator model logs are retained with their source fixtures. The combined-head Local gate uses the four pinned original comparison repositories and a Pro package built locally from the frozen firmware. The shipped native pair and consolidated ancestry were rechecked for this evidence checkpoint.

### Remaining engineering work

The 32-byte cell exposes INIT and two live performance parameters. A factory importer needs a distinct typed interface for physical parameters, full validation before applying any ordered stream, and preservation of both owners when a program is rejected. App parameter IDs 0 and 1 represent oscillator morphs. Passing them through the existing performance API changes pitch bend and damper. The independent oscillator prototype separates the two domains. The [event review](../../experiments/sound-hardware-v1/stock-dry/event-review.md) also records DnBOX's paired-note-off convention for overlapping presses from the same source.

For SuperJ6 the tested typed entry now carries all four AMP2 values intact on both owners. The importer still needs a normal caller, stable bank/program identity and support for the remaining original fields. The existing packed cell cannot carry the complete programs. Complete original preset captures also need the matching iOS AU host or the authenticated 14/56 comparison archive. The delay comparison documents two distinct audible original histories, the measured available stock storage and the results of a [2,748-byte per-owner approximation](../../experiments/sound-hardware-v1/synthone-delay/bounded-delay.md) that failed on the actual wet mixes. An exact on-demand replay regenerated 44,100 delayed sample words in two zero-feedback scenes. Its cold request needs 22,136 frames per owner before the first requested tap. The [measured replay](../../experiments/sound-hardware-v1/synthone-delay/deterministic-replay.md) exceeds the callback budget. Complete-preset comparison and callback qualification on the full graphs remain open.

## Stock memory checkpoint, 2026-11-20

The [sound memory report](sound-memory.md) records six new physical links using the existing UI and normal stock linker. Playable Synth One with the shipped Juno leaves 4,160 B XIP on Elite and 4,168 B on Pro. The partial compact SuperJ6 variant leaves 1,864/1,872 B, or 1,928/1,940 B with the range dictionary. Each leaves 8,972 B raw pool, 780 B above reserve. Whole-lane S6 remains FAIL because complete independent instruments and insert histories are still absent. Source patches, attribution and native receipts are in the report.

The current lead task is to test the filter response on the original note recordings and verify both the factory parameter namespace and the working oscillator/control candidates. The complete branch gate and independent review are outstanding. The live base is `76040daa5b52d686a8b7b544296ba5d79610a95d`; GitHub and the local merge-base agree, and the no-rename diff contains no deleted paths. Scope and evidence prior to this checkpoint follow below.

## Independent SuperJ6 layer 1 attack field — 2026-11-19

The [SuperJ6 field adapter](../../experiments/sound-hardware-v1/sj6-id36-field-adapter/CURRENT.md)
now applies already-decoded app ID 36 (`neonattack`, main application position
34) to the first layer of the independent DnBOX candidate. It uses the existing
normalized attack conversion and physical setter. The caller must explicitly
establish independent, unlinked envelopes. Unknown or conflicting link state
rejects the complete attack batch without mutation. ID 119 remains raw-only.
The adapter adds no per-sample dispatch or persistent state.

The raw-only baseline reproduced the bug: at knob zero it stored the value but
left attack at 0.008 s instead of 0.002 s. Five normalized settings then matched
the direct physical path across all 10,240 stereo frames, including complete
part state; layer 2 remained audible and unchanged. Wrong-layer and linear-law
mutants failed. This establishes pre-note field application for one qualified
control. It does not establish linked-mode callbacks, the full preset importer,
original complete-preset audio, native size/cost or production activation.

## Measured stock fit and independent dry voice — 2026-11-19

Sole Sound ownership and existing `lane/sound` / draft PR973 are preserved.
The exact predecessor/partial-upload recovery completed before source writes,
as recorded below. Original engines are comparison references only. New sound
behavior uses existing DnBOX components and independently written adapters.
**Production Synth One remains silent and unresolved.**

### Actual native results

The separate local executor completed the original compact Elite pair, then
the four stock jobs allocated by Integ comment 6995594729. Stock results and
all four physical maps/command-log archives are published in
[`f291e916a9b61ae862899f0d26e21d42d76a7846`](https://github.com/gobbajogger/DnBOX/commit/f291e916a9b61ae862899f0d26e21d42d76a7846).
All 292 serial stock compilations, four links and 12 inspections exit0. Every
plan received fresh exact-input/toolchain/header/environment/capacity checks;
all owned local process groups were released and the allocation is consumed.

| Stock image | Load including marker | Above unchanged 1,024-byte XIP reserve |
| --- | ---: | ---: |
| Elite reference | 551,492 B | 156 B |
| Elite shared | 544,708 B | 6,940 B |
| Pro reference | 551,484 B | 164 B |
| Pro shared | 544,700 B | 6,948 B |

Sharing saves 6,784 bytes per stock model. The earlier compact Elite saving is
6,496 bytes; these different closures are not additive. Stock Pro does not
resolve unmeasured compact/full independent Pro fit. All four stock maps keep
pool raw free 8,972 bytes, 780 above 8,192 and 268 beyond the extra 512-byte gate
margin; ordinary RAM 1,100 free; XPOOL 0. Default development model 900 and the
empty BREAK bank stay explicit. Stock profiles still include silent S1.
[Detailed scope and exact evidence](../../experiments/sound-hardware-v1/shared_math/production_fit/README.md).

### Implemented dry-voice behavior and genuine failures

The [independent dry-voice package](../../experiments/sound-hardware-v1/dry_voice_followup/README.md)
now preserves four frozen variants. A passes 79/180 static notes; B 109/180;
C 109/180; D 138/180, 45/60 complete files. All full gates remain **FAIL**, with
zero nonfinite samples. D adds 29 passing notes relative to C and loses none;
all 45 zero-resonance notes remain byte-identical. D's worst RMS error is
4.832778 dB and worst spectral error 8.991580 dB, against unchanged 1/3 dB limits.
The selected 2 kHz/resonance 0.9 file now passes all three notes.

C corrects the delayed-feedback instability with centered feedback and adds
two history floats. Matched isolated filter observations show C compressing
signals where the original remains proportional at the three tested levels.
D replaces only that nonzero-resonance feedback with the existing primitive's
unfitted slope-five linear law and a direct implicit solve. It retains C's
complete r=0 branch, forward poles, coefficient preparation, histories, gain,
controls, two substeps and tail policy. No original algorithm was copied.

C/D use 160 host bytes per mono voice. Fresh two-owner tests each verify 8,192
sample words, 310 complete state checkpoints and 310 untouched-owner checks
under irregular blocks/live edits; 16,384 outputs are finite. Source reviews
and exact comparisons establish their bounded scope, not full instruments,
arbitrary modulation, target cost or production activation.

The remaining 42 static failures are at 64 Hz/resonance 0.9 for notes 36/84, and
8,000/22,050 Hz at resonance 0.5/0.9 for all three notes, across all three
velocities. Their exact reports remain failures. No tolerance, gain, alignment
or retirement rule was changed to hide them.

### Reference recovery and remaining qualification

All 60 static original files/180 notes were recovered exactly. Six zero-depth
dynamic pairs are authenticated aliases to static originals, providing 18
labels without new observations. Historical nonzero-depth paired PCM for 18
files/54 notes remains missing. A separately rebuilt original-only closure
reproduced the selected three-note baseline byte-for-byte, then produced one
new exact-contract active-filter original capture. Original engine source
stays isolated in comparison recovery evidence.

On this one new depth-1.2, cutoff-2,000 Hz, resonance-0.9 recording,
unchanged D passed all three notes after its static replay was byte-identical.
RMS/spectral errors in dB were 0.232227/0.682166 (36), 0.239112/0.767147
(60), and 0.196105/2.106154 (84), gain 1, zero nonfinite. This is three new
active observations, separate from the 138/180 static count. It does not
recover the historical paired files or qualify the 72-case modulation matrix.

The published comparison entry snapshots verified source/metric bytes, rejects
existing/symlink outputs, enforces exact note coverage and gain 1, and retains
mandatory checks under optimized Python. Original runners/failures remain
immutable evidence. All earlier exact envelope/component results are retained.

`scripts/gate.sh sound --post` remains unqualified and unrun. Target identity
still needs exact successful image/package pairs and a reviewed 909/808/BREAK/
FX/master driver. The available old 94197fc8 diagnostic is not that proof.
Full native callback timing, simultaneous complete S1/SJ6/FX memory, all 150
S1 controls/694 presets and 218 SJ6 controls/640 presets remain open. Web keeps
its shared RDC window; there is no fallback. No PSRAM, main/force/device,
Actions/Codex or #156 operation is introduced. PR973 remains draft.

## Active successor implementation — 2026-11-18

The user transferred sole Sound ownership from the length-limited thread
`6afe1680-84a4-83e9-99bc-62619d34b5d0`. Exact recovery finished before source
writes: all 690 staged paths and 342 reviewed files matched tree
`74fdcc425ba2e8bbf8d287ebaf3577d462ce048c`, with no deletions. The verified
preservation commit is `9be11840b11d256f22a9af19d290cfe96f31a429`, ordered parents
`b90faafd2de71063448beea67aa5d22938a5c64b` and consolidated
`76040daa5b52d686a8b7b544296ba5d79610a95d`. Existing `lane/sound` and draft
[PR #973](https://github.com/gobbajogger/DnBOX/pull/973) are retained.

The allocated separate local executor completed `compact-reference-elite-02`
and then `compact-shared-elite-03`, with every one of 74 compilation units
rebuilt for each member. The reference reproduced the 1,772-byte XIP reserve
shortfall. The shared-math physical link succeeded at **546,924 bytes**, saving
**6,496 bytes** and leaving **4,724 bytes above the unchanged 1,024-byte XIP
reserve**. Both retain pool raw free 8,972 bytes, only 780 above its 8,192-byte
reserve / 268 above the extra 512-byte gate margin. Ordinary RAM free is 1,100
bytes and XPOOL free is zero. [Actual results and evidence](../../experiments/sound-hardware-v1/shared_math/README.md)
are a partial-core resource measurement, not complete sound qualification.
Pro was not rerun; its prior 1,764-byte deficit remains open.

Implementation now includes an independently authored S1 envelope component
with 12-byte state, separate from SJ6's AHDSHR. Stage coefficients latch on
entry, sustain remains live, edits precede the sample, and retrigger retains
the current level. No allocation, transcendental calculation or shared mutable
state occurs in its sample routine. The component is not connected to the
production renderer. Its ten immutable scalar recordings total **379,288
binary32 outputs, all exact**, with five API/behavior tests, two input-integrity tests and four rejected
C mutants. A boundary-edit regression was added after independent review found
that an early decay latch survived the first four tests. All failed evidence
is retained. [Component contract and verification](../../experiments/sound-hardware-v1/component_followup/README.md)
explain the qualified scope.

Prepared-coefficient alternatives remain research-only. Existing fastmath is
exact on the recordings but differs for some legal physical controls against
the inferred mathematical formula. An independent integer proposal matched
5,360,032 tested coefficients, including subnormal boundaries, but still needs
native cost and broader original-behavior qualification. A recovered physical
range dictionary preserves complete state/audio in the host comparison; its
79-byte prior object saving is not an additional measured image saving.

Focused dispatcher tests `synthone-envelope-dnbx`, `fastmath`, and
`fastmath-ubsan` pass on the current source. The full firmware suite and
`scripts/gate.sh sound --post` have **not** run: exact production Elite/Pro
links, target regression/timing and the next local allocation remain open.
PR comment6994424328 requests the next bounded allocation. The two-link slice
is consumed, Web owns its next shared RDC window, and no RDC fallback is taken.

**Production Synth One is still silent and unresolved.** A complete independent
S1 profile still needs the common oscillator/VCA/four-pole filter path, all 150
controls, 694 presets, FX and two-part ownership; SJ6's 218 controls/640 presets
and full sound/FX qualification remain separate. Existing S1–S6/common FAILs
are not converted to PASS by these component results. No production activation
or consolidated merge is admitted. No PSRAM, main/force/device writes,
Actions/Codex work or #156 mutation is performed.

## Recovered predecessor checkpoint — 2026-11-18

This thread is the sole Sound production owner after consuming Dot Sound's frozen handoff. Preserve `lane/sound` and draft PR #973. The handoff archive SHA-256 is `4cc1b62ece0685eaa29b2c7ff86e7b58a4760a50b829bf27a18c38f6ef1bd773`; its live head was rechecked as `b90faafd2de71063448beea67aa5d22938a5c64b`. Consolidated `76040daa5b52d686a8b7b544296ba5d79610a95d` was merged in locally without conflicts; the existing lane history and files are preserved.

Latest user direction overrides the older implementation allowance: original SuperJ6 and Synth One engines are comparison references only. New code uses existing efficient DnBOX DSP and independently written behavior adapters. No external Codex or Actions jobs, PSRAM, device writes, main writes or force-push are authorized. There is one Sound writer.

The requested research logs are retained under RULES.md v2 rule 13; that rule and the current evidence-delivery request take precedence over the generic PR template's log exclusion. Firmware payloads and app installers are not new package contents.

The new [hardware-first research package](../../experiments/sound-hardware-v1/README.md) contains source, executable tests, independent review and retained failures. It is not wired into the production runtime. The [owner report](../sound/hardware-first-owner-report.md) records the complete evidence and gaps.

- Independent original-amplitude subset: 5,120 exact-timestamp endpoints pass the predeclared 0.001 threshold; worst error 7.95e-7. Shifted timing, linear contour and wrong velocity negatives reject. A real allocator defect was reproduced and fixed.
- Replacement RAM: both full part owners and existing selected FX remain simultaneously allocated. Candidate adapter plus stock chorus is 2,892 B inside the existing 3,720 B backend union; no pool growth is claimed or required by this partial core.
- Replacement flash: the compact reachable `-Oz` image is 553,420 B Elite / 553,412 B Pro. Physical overflow is 748/740 B; required-reserve shortfall is **1,772/1,764 B**. S6 remains FAIL, before the missing control/source/FX work.
- Callback: actual contract is 512/44,100 = 11.609977 ms. Candidate engine/event slice was measured, including event/control bursts and omitted render-counter work. Highest independent observed host maximum is 9.785132 ms. Full native callback, other simultaneous owners, stack high-water and xruns remain unmeasured.
- Synth One remains a silent production slot. The new common core does not match its four-pole transfer or VCA-before-filter topology. The historical original-code renderer remains reference-only; its existing fidelity results are not new-engine admission.

The production gates S1–S6 and common integration gates remain open or failing. No merge into consolidated is admitted by this checkpoint. Do not repeat the exhausted history-codec or compact-hall sweeps. The next work must reduce measured shared-code/adapter cost and independently qualify missing original behavior, with the whole simultaneous image accounted for.

## Preserved pre-handoff record

The current draft is PR #973 on `lane/sound`. This update restores the original generic Synth One renderer inputs and keeps its independent reference implementation separate from the experimental harmonic fixture.

## Checks on the restored source

- All 45 focused Python tests passed. Run `python3 -m unittest discover -s tests/host -p 'test_synthone*.py'`. Before restoration, three tests failed and seven raised errors.
- The portable bundle restored all 97 files. Its archive SHA-256 is `885f793f13845b74ded63151a65be75fdfadea79fc7696655397ea07bd498306`. The bundle and PCM manifest come from `f50d5c14`. The tuning fixtures come from `66b1ffef`.
- The pre-FX adapter rendered 576,000 frames across 12 scenes. There were zero differing channel words and zero render heap calls. The scalar control comparison checked 957,750 targets and found zero differing words.
- The generic factory comparison rendered 694 presets over 66,624,000 frames. There were zero differing or nonfinite words. Both outputs used the same FX implementation. All 1,388 output filenames, byte counts and SHA-256 hashes matched the historical PCM manifest. The commands in the portable input guide reproduce the test. [Factory results](../sound/restored-factory-replay.json) · [PCM manifest](../sound/v2-synthone-factory-pcm-manifest.json) · [Input guide](../../assets/test-inputs/synthone/README.md)

## Checklist

| Item | Status | Evidence or remaining work |
| --- | --- | --- |
| S1 | FAIL | The restored timing bench measured all 694 programs and missed 3,602 wall and 2,629 thread-CPU deadlines in 520,500 callbacks. All output words were finite. Profile the worst cases with the original floating-point settings. |
| S2 | FAIL | Historical harmonic A/B tests reported 492 bit-identical programs and 202 approximations. A current per-program spectral and level report remains open. |
| S3, S4 | FAIL | SuperJ6 control/FX coverage and comparison with the original captures have yet to be qualified on this lane. |
| S5 | FAIL | The replay covered 694 programs. Complete the independent control-to-parameter mapping check for the required banks. |
| S6 | FAIL | Last matched experimental pre-FX build exceeded Elite flash by 30,676 bytes and Pro by 30,668. Two-part pool margin was already 4,876 bytes below reserve before FX. Next measure retained code and independent voice histories against a matched stock build. |
| S7 | PASS | Case-insensitive whole-word Git scan under firmware, tests and web found zero hits on the restored source. |
| C1, C3–C6 | FAIL | Full firmware gate, independent review and overall checklist remain open. PR stays in draft. |
| C2 | PASS | The existing lane includes consolidated commit 89b40a93. No consolidated file was deleted. |

## Moved file

The experimental `firmware/src/dsp/synthone/scheduler/oracle/state.hpp` moved to `tests/fixtures/synthone-storage/oracle/state.hpp`. The generic renderer refuses to overlay any runtime source on its independent reference. Moving the fixture preserves that check and lets the normal renderer run against the pinned source.

## Timing and memory measurements

The callback benchmark and its original deadline contract were recovered from `55e1288d`. The untraced 48 kHz host test used 128-frame blocks. The full 694-program result was DEADLINE_FAIL. [Timing receipt](../sound/restored-callback-result.json)

At 44.1 kHz the four original ten-second delay histories alone allocate 7,056,016 bytes. In a fresh component test with pseudorandom input and a two-second delay, reducing the ring from ten seconds to 5.0001 seconds changed an output word at frame 220,528. An earlier minimum-size native build with link-time optimization still exceeded Elite XIP by 25,528 bytes and Pro by 25,520. The current build flags and effect behavior are preserved.
