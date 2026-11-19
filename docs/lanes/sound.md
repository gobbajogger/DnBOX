# Sound lane

## Independent dry voice and next stock fit preparation — 2026-11-19

The independent source work continues after published envelope/shared-math
commit `e0ef1994a91abef619c755f9428e44c8c9f064c4`. The new
[dry-voice package](../../experiments/sound-hardware-v1/dry_voice_followup/README.md)
contains two frozen implementations, the exact recovered original PCM contract,
all numerical reports, independent audit and a portable comparison/recovery
entry. Original engines remain comparison-only. A passes79/180 static notes;
B passes109/180, including all45 zero-resonance notes. Both full gates are
**FAIL**, with zero nonfinite samples. B's worst resonant error is45.163240 dB
and includes sustained post-note-off feedback oscillation. No tolerance, gain,
alignment or tail rule was changed to disguise it.

The new152-byte isolated voice combines stock raw saw, squared velocity,
explicit component gain, the independently written S1 amplitude/filter
envelopes, VCA before filter and an existing DnBOX four-pole primitive. It is
not production activation or full control/preset coverage. A bounded analytic
diagnosis identifies the delayed-feedback instability and proposes one
centered implicit correction; its separate implementation/comparison follows.

Independent audit found the recorded A/B bytes and all120 report/log sets
consistent, then found four historical harness weaknesses: absolute scratch
paths, removed assertions under optimized Python, incomplete resume validation
and metadata overwrites. Those originals remain immutable. The new entry
stages verified source/metric snapshots, uses explicit checks, requires exact
note coverage and fresh output, and does not resume. Four integrity regression
tests pass normally and under optimized Python. An optimized-Python selected
B replay reproduces candidate/paired PCM and metric JSON byte-for-byte.

The original archive has60 static physical files/180 notes. Further reference
recovery is separate from candidate implementation; no active-modulation or
full-preset qualification follows from those recordings.

The [next stock native plans](../../experiments/sound-hardware-v1/shared_math/production_fit/README.md)
are prepared for Elite reference/shared and Pro reference/shared,73 official
compile units and357 pinned inputs each. They are **NOT_RUN / ALLOCATION_REQUIRED**.
PR973 comment6994965759 is the concrete four-link request; no response granting
it was present at the latest read. The old two-link allocation is consumed.
Web retains the next shared RDC window; there is no fallback. The available
old-image diagnostic runner does not qualify current target audio identity.

`scripts/gate.sh sound --post` remains unqualified and unrun. The actual prior
Elite partial-core improvement is retained below; stock/current-production
Elite/Pro, target regression/deadlines and complete instrument fit are open.
**Production Synth One remains silent.** PR973 stays draft and S1–S6/common
FAILs remain open. No PSRAM, main/force/device/Actions/Codex/#156 operation occurs.

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
