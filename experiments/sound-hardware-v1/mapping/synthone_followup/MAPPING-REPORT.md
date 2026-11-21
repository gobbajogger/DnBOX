# Synth One bounded mapping result

## Decision

The existing independent DnBOX candidate is useful infrastructure for a new Synth One profile, but **it is not currently an accepted Synth One dry voice**. The current signal order, filter transfer, ADSR timing, waveform numbering, physical cutoff range, and gain normalization differ. Selectable raw oscillation and squared velocity solve only two parts of the mapping.

The next useful isolated unit is a six-note S1 dry profile with one raw saw, independent amplitude and filter ADSRs, prefilter VCA/velocity, the physical filter-envelope relationship, and a newly written/approved stock-derived four-pole filter candidate. The source-aware historical implementation must remain a reference. Full S1 modulation, oscillator coverage, FX, lifecycle, preset application, and hardware gates remain separate unfinished work.

This folder contains no production edit. It does not rerun the previous SuperJ6 12-sample/42-capture work, full 694 renderer, codec sweeps, Hall sweeps, device work, PSRAM work, external Codex, Actions, Git operations, or posts.

## Delivery boundary

**Give a fresh implementer only `implementer_brief/`.** It contains:

- `BEHAVIOR-BRIEF.md`: physical units, routing, bounded scope, component options, and remaining gates.
- `s1_control_catalog.json`:150 public identities and exact binary32 bounds/defaults, 46 smoothing memberships, preset field mappings, and command statistics.
- `behavior_fixtures.json`:100 physical cutoff targets, 5 velocity targets, historical grids, dry scene timing, and required negative controls.
- `envelope_observations.json` and `s1_envelope_observations.f32le`:264,600 newly observed original-component outputs, metadata and hashes; no original recurrence or engine code.
- `verify_contract.py`: packet-only integrity/numeric checks. It does not read original source.

`reference_review/`, this report, and `results/input_provenance.json` are for the reference/review owner. They record original-source locations and reference-side executable observers. They are not implementation source. Previous `hc.c`, `shaped_env.h`, `j6-envelope/compact.h`, scheduler/oracle/source-control ports, and frozen original Soundpipe engine code are excluded from the safe packet.

## What the historical evidence actually supports

| Evidence | Verified meaning | Does not establish |
| --- | --- | --- |
| `physical-gain/SUMMARY.json` | 180 complete cases: velocities 20/100/127 × cutoffs 64/500/2000/8000/22050 × resonances 0/.25/.5/.9 × notes 36/60/84; fixedgain 1; max RMS0.00013075611673406606 dB, spectral 0.005425824870207458 dB | New candidate parity; all oscillator/control settings; full presets; nonlinear high-drive domain |
| Latest `filter-modulation/SUMMARY.json` | 72 complete cases: depths 0/.5/1/1.2 × cutoffs 500/2000/8000 × resonances 0/.9 × notes 36/60/84; max RMS0.0003255912716613262 dB, spectral 0.02380014809445087 dB | Full LFO/modulation matrix or production filter implementation |
| Historical prepared cache | Preserved exact outputs over paired 1,024,000-frame sequences; per-voice prepared coefficient organization is a useful design pattern | Hardware timing or permission to copy the reference-aware implementation |
| Factory reconstruction | 694 programs, 66,624,000 frames, zero differing/nonfinite words; 48 kHz, 96,000 frames per program | Independent FX proof: both sides use common original FX; two-second scenes do not cover long tails; no new candidate or target admission |

The matrix audit recomputes row counts, Cartesian completeness, and the published 1 dB RMS/3 dB spectral gates from saved numeric rows. No old audio was rerendered. The latest filter history retains its RED coefficient result and the earlier incorrect normalized adapter fixture; these were not erased or passed off as a first-attempt success.

The factory result is bound to the correct artifacts. Its `factory_commands_sha256` refers to `factory-build-commands.json`; `frontend_commands_sha256` refers to the binary `preset-commands.bin`. Our checker initially compared the former to the preset JSON and failed. It now verifies both correctly named files and reserializes all JSON commands to exactly the accepted binary. This was an audit-field interpretation error, not a corpus mutation or weakened hash.

## Factory coverage and namespace

All 12 raw bank SHA256 values match the frozen manifest. There are 694 entries, 96 direct public field writes plus 48 sequence-element writes per entry, 144 unique ordered parameter writes per entry, and 150 declared S1 parameter IDs. IDs 18,31,33,140,144,145 are not written by the factory loader and retain separately owned/default behavior. Their absence is not evidence that they are unnecessary.

The physical single-saw test patch has **zero intact factory matches**. The explicit filter funnel is 480 bandlimit-off programs → 431 also LP → 58 also oscillator 1 saw → 3 also oscillator 2 volume zero → 0 also sub volume zero. This is the exact restricted patch used by the historical experiment, not an assertion about every possible signal-equivalent special case.

| Corpus observation | Count |
| --- | ---: |
| Bandlimit disabled/enabled | 480 /214 |
| Low-pass/band-pass/high-pass selection | 598 /60 /36 |
| Nonzero second oscillator volume | 629 |
| Nonzero sub/FM/noise level request | 589 /574 /444 |
| Arp enabled | 338 |
| Mono enabled | 326 |
| Nonzero glide | 331 |
| All 12 LFO routes disabled | 80 |
| Nonzero phaser/autopan/widen setting | 359 /342 /214 |

These are source-derived command statistics, before dependent setter side effects and smoothing. Nonzero requested controls alone do not prove audibility in every patch. Nine raw requests exceed physical metadata bounds: one noise 0.5 request versus 0.25 maximum, and eight Starter Bank index 0 values below the minimum for compressor ratios/makeup or LFO rates. Preserve raw preset data and reproduce the proper setter behavior; generic 0–1 clipping would destroy legitimate units and bounds.

The current production sound registry exposes S1 `INIT` with PBend/Damper, while the existing full adapter describes 150 physical values. That UI/serialization gap remains. The new candidate retains 218 unrelated SJ6 raw IDs and binds none automatically. Neither data retention nor shared numeric ID values establish S1 controls.

## Fresh executed diagnostics

`results/candidate_probe.json` came from compiling the existing independent candidate read-only with a small local harness. It proves:

- With sustain 0.25 versus 1, output ratio is 0.25 while the corresponding filter state ratio is 1.
- With velocity 20 versus 127 in squared mode, output ratio is 0.024800051 while filter state ratio remains 1.
- Candidate physical cutoff 22,050 Hz is rejected with status −2.

Thus amplitude/velocity are postfilter in that candidate. No speculative listening inference is needed for this routing fact. The candidate also has two independently filtered layers, whereas a complete S1 note normally sums oscillator components into a common note envelope/filter path. Its per-layer `/6` scaling and centered linear pan introduce additional output normalization absent from the historical dry reference. Its SJ6-oriented curved AHDSHR is not the S1 ADSR target.

The original S1 ADSR component was executed in isolation with 3 static settings at 44.1 kHz, gate-off at one second, total two seconds each. All 264,600 outputs are finite. For 50 ms nominal attack, peak occurs at frame 6092≈138.14 ms; for 200 ms nominal attack at frame 24370≈552.61 ms. Samples advance at frame 1, unlike the separate SJ6 capture's 16-frame timing. These numeric outputs are provided to the fresh worker. This experiment does not prove app-level restart, dynamic edits, allocator retirement, or full audio. The source SHA is pinned below; the original implementation is not copied into the packet.

## Stock reuse beyond Juno

| Existing block | Observed state / dependencies | Suitable reuse direction and mismatch |
| --- | --- | --- |
| Chicago MP4 | 60 B host filter state; four cascaded stages plus feedback history; prepared ramps; logical A/E table interface with 16,385 entries each | A four-stage starting point and coefficient cache interface. Chicago resonance/postgain/kick and transfer remain different. Logical full raw tables total 131,080 B; that is not measured marginal linked cost or a proposal to allocate duplicates. Current shared/packed provider must remain owned by its production owner. |
| Miami MS20 | 72 B host filter state; prepared A/resonance/noise values, separate history and noise seed; 129 entries each in A/B/R/cap | Reuse management/immutable table access only where authorized; MS20 feedback/nonlinearity is not the S1 four-pole filter. Raw logical table storage 2,064 B excludes pitches, code and other state. |
| Master SVF | Two histories per channel, shared prepared coefficients; whole master object 664 B host including compressor/limiter | Extracting/independently wrapping the stock filter interface could avoid dragging in unrelated master state. Two-pole SVF is not S1 LP equivalence. |
| Drum/FX biquad | Declared 9 floats = 36 B, including 5 coefficients and 4 histories; LP/HP/BP; fixed 44.1 kHz | Existing generic filter building block for experiments or auxiliary paths. Needs S1 frequency/Q, nonlinear and mode-switch qualification. Its full header could not be compiled in this read-only checkout because generated`x0x_drum_tables.h` is absent; 36 B is declaration accounting, not a falsely claimed compiled type result. |
| Bus one-poles / Juno filter | Small existing states and code | Useful DC/tone/smoothing utilities. They do not supply the required four-pole voice transfer. |

The Chicago/Miami production adapters currently include `experiments/original_synth` files. This task only inspected these existing DnBOX components and their interfaces. A fresh worker's code allowlist must explicitly identify approved stock files; do not hand it the entire original-source tree. This report recommends component reuse opportunities, not indiscriminate original-code reuse or transfer equivalence.

The physically smallest meaningful S1 architecture initially has six voice states with one amp envelope, one filter envelope, one raw oscillator, and one shared-per-note filter. Add the second independent oscillator into that same note mix when qualified. Cache only immutable or prepared coefficient work; each voice/part/tail retains independent histories. The production global delay, selectedFX arenas, and shared workspaces are already owned. Sharing code does not free their simultaneous mutable storage.

## Verification, provenance, and limits

Executed: metadata/corpus/hash audit; 252 saved matrix-row checks; 264,600 fresh numeric ADSR observations; existing-candidate routing/range probe; existing-component host layouts; safe packet test with 8 negative-control checks. No candidate full-audio parity, 694-program candidate pass, new target timing, target flash fit, or listening acceptance is claimed.

`results/input_provenance.json` records every inspected source's absolute path, byte count, and SHA256. The upstream S1 pin is`6466a3715c96b7ecf1dd255a218cbd571408a314`. Selected anchors:

| Input | SHA256 |
| --- | --- |
| Frozen factory physical descriptor |`2180c7385ce792eb5f97769301ab83e11998a97d35af768d38c1e6922f1b4b38` |
| 694 result |`d98d682baee662fda95d096e6ddb3f40f2b5c47488b0d3e36ea7a57e5d5fce66` |
| 180 physical-gain summary |`0aca21ecbbc4a405d48516170f5c3be4497e9bf232515da7285ce1b95f199886` |
| 72 filter-modulation summary |`dece44e10f6b301296bd8a634ce411c471c26aebab5e3298440d4537e5d402ca` |
| Original ADSR component, reference only |`816310221f5ce2e04751d7f284a61cc9057cbabf44deef7366a085c57ca310dc` |
| Independent candidate tested here |`9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa` |

Remaining barriers: independent S1 envelope/filter implementation, raw phase/morph/bandlimit behavior, two-oscillator gain/tuning, all modulation and control laws, BP/HP/lifecycle, source-compatible sequence/mono/damper events, complete stereo FX, preset persistence/UI, native state/link/resource floors, full callback deadline evidence and listening. The historical direct adaptations remain comparison material only.
