# Sound lane

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
