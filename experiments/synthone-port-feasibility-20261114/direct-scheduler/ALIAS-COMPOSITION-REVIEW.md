# Independent review: exact aliases composed with the direct scheduler

**Verdict: ACCEPTED for this pinned source/host experiment only.** Both P2 findings are resolved. No actionable findings remain within the reviewed boundary.

## Exact reviewed identity

- Final 74-file freeze SHA-256: `920651fb94d6d0cb303af9db19bd78109b0ad4e4594c4b6f5fa3919caf788f06`.
- Source-manifest SHA-256: `03e8c7dd175027352fce18266c2c2c1677e2b2cdd0a9e3396f9aea1b011412ed`.
- Baseline #941: `bb6745d6f794d4befe6d03610e51400891bb25e1`; baseline manifest `d138334eb8a6eb800b1022a8d58b828aafe149d2408d1e3977184bdd46c8d58c`.
- Alias provenance #936: `591774bd9c18ba82ba405abe1a52e74633f0e8a7`.
- Public upstream AudioKitSynthOne: `6466a3715c96b7ecf1dd255a218cbd571408a314`.

All final frozen files and source-manifest entries were independently rehashed before and after validation. Producer and reviewer compiler closures match the exact pinned source/dependency bytes. Scheduler/runtime, original-source oracle arithmetic, DSP/control dependencies, native runner and native toolchain manifest remain byte-identical to #941. This successor changes the host fixture/runner and associated evidence, not runtime DSP arithmetic.

## Finding closures

**ALIAS-SCHEDULER-P2-1: scheduled renders did not select the duplicate rows. CLOSED.** The initial fixture inherited disabled bandlimiting; its standalone oscillator copies did not establish scheduled alias access. The corrected fixture uses real no-note Scheduler/Oracle processing to warm cold controls, explicitly enables bandlimiting, and selects the alias region through real source APIs. Per-rate assertions now require scheduled rows 48/49. Three full-scheduler wrong-row 48 controls require finite, nonzero original PCM and a genuine PCM difference. No control outputs or port histories are manually forced.

**ALIAS-SCHEDULER-P2-2: the changed pitch set made the release event inactive. CLOSED.** The final fixture retriggers existing pitch 127 through the direct-on API at 20,000 Hz, releases it, and asserts that both graphs retain pitch 127 in slot 5 at stageRelease. It then requires alias selection by that actual releasing oscillator. An independently compiled inactive-off mutation fails the release-stage assertion after successful linking.

Independent review-only instrumentation at the real morph source's selected-pointer site confirms, at each of 44.1, 48 and 96 kHz: 3,436 scheduled runtime morph calls, 768 row 48 and 1,536 row 49 selections, with selected pointers matching the intended row 44/45 aliases. During each 63-frame release block it additionally verifies the actual voice's retained pitch and stageRelease, and observes 63 alias calls. This is host software-source evidence, not target execution or a hardware-load trace.

## Fresh independent verification

O2 and Os each PASS with:

- 7 cases; 7,687,077 checks; 54 events.
- 213,044 exact logical-word comparisons; 7,371 standalone oscillator boundary cases.
- 1,701 positive scheduler PCM frames; 2,571 canonical state comparisons.
- 376,200 no-note warmup frames with block PCM/state equality.
- 10,308 positive scheduled morph calls; 2,304 row 48 and 4,608 row 49 selections; 189 releasing-alias calls.
- 7 protected-write probes; 3 standalone wrong-alias controls; 3 scheduled wrong-row 48 controls over 1,536 frames.
- No heap operation observed during guarded DSP/control actions.

The fresh original full O2 fixture also PASSes unchanged metrics: 54 cases, 55,654,337 checks, 18,162 frames, 19,302 snapshots and 648 events.

The fresh default sanitizer run **FAILS** because LeakSanitizer cannot operate under ptrace. The exact same binary with leak detection disabled **PASSes** under ASan/UBSan/float-cast-overflow instrumentation, matches the final alias metrics and has empty stderr. This is not a leak pass.

Independent compiled controls reject absent warmup, disabled positive bandlimits, an incompatible active-voice setup, inactive note-off, removal of the negative row 48 corruption and disabled negative-case bandlimits. An additional scheduled row 49 corruption also produces a real PCM difference. Prior failed candidates and both original P2 reports remain preserved, not relabeled as passes.

## Payload, comparator and limits

Independent reconstruction from 59 authenticated upstream source records and bytewise deduplication confirms only logical 48->44 and 49->45: 52 logical rows, 50 unique guarded morph rows, 819,400 bytes, packed SHA-256 `5e174ae6d1e4c13726831d505fe4c32609c87ff78d80b6b2b9264a0d3464e595`. The original guarded payload SHA-256 is `0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5`. The separate shared host sine remains 16,388 bytes.

The runtime consumes a separate protected packed allocation; the oracle retains distinct original rows. Address normalization excludes only intentional physical table addresses. Exact descriptor metadata, independent binding assertions, all logical word bits, PCM and canonical state remain checked. The original 54-case fixture retains raw table-address capture by default. The oracle is authenticated original scheduler-body extraction but intentionally shares the common voice/DSP primitives; this is not an independent whole-instrument implementation.

Acceptance establishes only this source/host alias-to-scheduler composition. It establishes no target asset representation or placement, complete executable/firmware link, current Juno coexistence, target DSP/fidelity/workload/CPU, storage or memory fit, whole-engine/global-FX sound, target sine regeneration, device or release qualification. No Git delivery, firmware publication or device operation is asserted by this receipt.
