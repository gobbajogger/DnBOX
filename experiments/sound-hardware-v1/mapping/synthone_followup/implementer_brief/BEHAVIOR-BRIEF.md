# Synth One behavior brief for an independent DnBOX implementation

## Scope and inputs

Implement a bounded Synth One **dry voice profile** using approved existing DnBOX primitives and newly written code. This packet contains public parameter identities, numeric observations, and externally visible routing requirements. It contains no original engine source, copied recurrence, original waveform asset, or previous source adaptation. The reference owner retains those inputs separately. Do not open the sibling `reference_review` directory or original archives to implement DSP.

The current independent DnBOX candidate supplies six allocated notes, two independently phased layers, explicit raw/BLEP oscillator modes, separate envelopes, and selectable linear/squared velocity. These are useful infrastructure. Its current envelope shape, signal order, filter, waveform indexing, and parameter namespace are not an accepted Synth One profile.

The immediate unit is one raw saw, independent amplitude and filter envelopes, physical controls, and a low-pass voice filter. It is a component experiment. **It covers zero intact programs from the 694-preset corpus.** Preserve the full preset data separately, and do not advertise unsupported fields as implemented controls.

## Identity, units, and control storage

Synth One has **150 parameter IDs, 0–149**. SuperJ6's 218 app IDs belong to a different namespace. `s1_control_catalog.json` gives each ID, name, binary32 minimum/default/maximum, public preset field, smoothing membership, and factory command statistics. A namespace discriminator or separate typed interface is necessary; storing an arbitrary scalar does not implement its DSP behavior.

| Controls | IDs | Physical contract relevant to this unit |
| --- | --- | --- |
| Oscillator morph positions | 0, 1 | 0–1; endpoint 1 is saw in the frozen Synth One control contract |
| Oscillator balance, levels | 2, 5, 6 | Balance and component levels 0–1; oscillator 1 contribution is `(1-balance) × level1` for the selected dry subset |
| Oscillator semitone offsets | 3, 4 | −12 to +24 semitones |
| Sub/FM/noise levels | 7, 10, 12 | Must be zero for this unit; noise maximum is 0.25, even though one raw preset requests 0.5 |
| Cutoff, resonance | 16, 17 | 64–22,050 Hz and 0–0.98; physical units, not a generic normalized pot |
| Filter envelope amount | 19 | 0–1.2; 1.2 is a valid value and must not become 1 |
| Filter ADSR | 22–25 | A: 0.0005–2 s; D: 0.005–2 s; S: 0–1; R: 0–2 s |
| Amplitude ADSR | 26–29 | A: 0.0005–2 s; D: 0.005–2 s; S: 0–1; R: 0.004–2 s |
| Master level | 32 | 0–2; excluded from the dry voice comparison |
| Filter type | 115 | 0 low-pass, 1 band-pass, 2 high-pass; this unit supports 0 only |
| Parameter smoothing half-time | 144 | 0.000001–0.99 s; separate from note glide ID 21 |
| Oscillator bandlimit enable | 146 | Explicit choice; this unit requires 0 |

Only IDs 15, 47, 36, 43, 140, and 147 have the dependent normalized API in the frozen control interface. They concern LFO rates, autopan, delay time, pitch bend, and arp/sequence tempo. Their rate/tempo/payload coupling is outside this unit. Do not apply the SuperJ6 `16*u^3.33` rate or `0.002+15.998*u^3.33` attack laws to Synth One.

The full reference smooths 46 parameters. Cold smoothing starts below its target and affects early notes. The historical dry tests used the minimum smoothing half-time and rendered 176,448 warm-up frames before note-on. That fixture choice establishes a settled-control experiment, not full automation behavior. A production profile needs explicit current/target state and timing for all selected controls.

## Dry signal behavior

For this unit, one oscillator is audible: oscillator 1 at saw endpoint, oscillator 2/sub/FM/noise levels zero, no modulation routes, arp off, no glide, and LP selected. Use one amplitude envelope and one independent filter envelope **per note**. The two oscillator inputs of a future complete S1 voice normally feed a common voice envelope/filter path; two separately filtered candidate layers are not automatically that architecture.

The required ordering is: oscillator/component gain and velocity → component sum and amplitude envelope → voice filter → voice sum. Post-voice FX and master gain follow separately. Putting the amplitude envelope or velocity after the filter changes excitation, stored energy, resonance, and nonlinear behavior. A constant output gain cannot correct that change.

Velocity is squared: for MIDI velocity `v > 0`, the selected oscillator amplitude factor is `(v/127)^2`. Numeric fixtures include velocities 1, 20, 64, 100, and 127. At velocity 20 the factor is approximately **0.024800051**, not 0.157480315. Velocity zero remains a note-off event. This does not redefine SuperJ6's separately observed linear velocity behavior.

Keep absolute gain explicit. The existing candidate additionally divides layer output by 6 and applies a half gain to each channel at center pan. The historical dry reference duplicates its mono voice output into both channels without that candidate normalization. A physical S1 component gain must therefore have an explicit, tested channel mapping; an arbitrary after-the-fact comparison gain is not accepted. The later full sub/FM/noise paths need their own velocity/gain tests rather than inheriting the selected oscillator assumption.

Raw oscillator mode is deliberate. BLEP/other anti-aliasing should not be silently added when ID 146 is zero. The candidate's waveform value `2/3` selects its raw saw, while value 1 selects square. An ID 0 value 1 must therefore go through a semantic mapping. Endpoint agreement alone does not establish matching phase, interpolation near the wrap, morphing, or enabled-bandlimit behavior. The historical reference-aware spike's phase-sensitive comparisons showed that these details matter at low cutoff/high resonance; its implementation is not reusable here.

## Filter envelope: physical targets

Let `C` be the current base cutoff, `D` the current filter envelope amount, and `E` the filter envelope level. With all filter LFO routes disabled, the observed target is:

`cutoff_hz = clamp(C * (1 - D * (1 - E)), 64, 22050)`

This is a control relationship, not a filter implementation. Examples at base 2,000 Hz:

| D | E | Target Hz |
| ---: | ---: | ---: |
| 0 | 0 | 2,000 |
| 0.5 | 0 | 1,000 |
| 1 | 0 | 64 |
| 1 | 0.4 | 800 |
| 1.2 | 0.4 | 560 |
| 1.2 | 1 | 2,000 |

An additive Hz excursion is a different control law. Candidate cutoff validation currently stops at 20,000 Hz and rejects the legal S1 maximum 22,050 Hz. The profile must represent the full requested physical range even when the chosen stable DSP internally handles the Nyquist boundary specially. Do not quietly reduce the requested range.

The LP reference has four-pole behavior and nonlinear level dependence. The current bounded two-pole candidate is not qualified. A fresh four-stage or other approved stock-derived implementation must be measured at the actual cutoff/resonance/input levels. Cascading two existing two-pole sections is an experiment, not evidence of equivalence. Band-pass/high-pass behavior and live type changes remain additional units; the reference keeps all three filter histories advancing during playback, so a mode switch cannot assume a cold state.

## ADSR numeric observations

The supplied binary file records **264,600 scalar outputs** from three frozen Synth One ADSR component runs at 44.1 kHz. Each case has a gate-on at frame 0, gate-off at frame 44,100, and 88,200 frames total. It is a behavior target; do not copy any reference implementation. `envelope_observations.json` gives binary layout, SHA256, settings, offsets, peaks, and selected sample values.

| A / D / S / R | First output | Peak frame / time | Last pre-release output | Output 0.5 s after note-off |
| --- | ---: | --- | ---: | ---: |
| 0.05 / 0.005 / 0.8 / 0.05 | 0.0007555485 | 6,092 / 138.14 ms | 0.800006568 | 0.00003630322 |
| 0.2 / 0.3 / 0.4 / 0.5 | 0.0001889467 | 24,370 / 552.61 ms | 0.532695115 | 0.195920885 |
| 0.0005 / 0.005 / 0 / 0.004 | 0.0727998018 | 60 / 1.36 ms | about 1.54e−43 | about 1.23e−43 |

The nominal attack value is not the time at which the peak is reached. Outputs advance every sample; the SuperJ6 16-frame AHDSHR observation is not this envelope. There is no explicit hold or release-hold control in these S1 ADSRs. The component is used for both amplitude and filter envelope in the frozen voice reference, but the active filter path still needs its own audio comparison.

These traces use static parameters. They do not qualify retrigger, live segment edits, voice stealing, silence detection, denormal handling, or retirement. In particular, the low-level component may approach a tiny nonzero value; voice lifetime must be verified from event/audio behavior rather than inferred from exact-zero amplitude.

## Existing DnBOX reuse opportunities

Reuse immutable code/tables and coefficient preparation where authorized; keep each concurrent voice, part, and FX tail's mutable state separate.

| Existing component | Useful part | Boundary |
| --- | --- | --- |
| Stock Juno/new candidate | Fixed voices, raw/BLEP mode choice, oscillator state, note infrastructure, finite input validation | Its current post-filter VCA, two-pole transfer, SJ6 envelope, waveform numbering and gain normalization are not S1 semantics |
| Existing Chicago filter path | Four-stage organization, prepared coefficient/ramp interface, same-value cache invalidation patterns | Existing filter transfer, coefficient tables, resonance compensation and kick behavior are Chicago-specific; source allowlist must be explicit before any extraction |
| Existing Miami filter path | Separately owned filter history, small prepared parameter set, modulation preparation | MS20-style topology, saturation and noise behavior are different; it is not a four-pole S1 substitute |
| DnBOX master SVF | Compact multimode structure, per-channel histories and shared coefficient preparation | Two-pole response; extracting the whole master object would also bring unrelated compressor/limiter state |
| DnBOX drum/FX biquad | Existing LP/BP/HP coefficient and tick building block | Frequency/Q law and numeric transfer need calibration; fixed 44.1k assumptions must remain explicit |

The smallest useful architecture for this unit is one independently implemented S1 profile inside the existing fixed part owner: six note states, one raw oscillator initially, one amp ADSR, one filter ADSR, and one shared-per-note four-pole filter state. A later second oscillator can share that note's amp/filter path, with independent phase and tuning. Keep physical parameter storage separate from DSP history and existing SJ6 identities. Store prepared coefficients per voice; share immutable coefficient code. Recompute when effective modulated cutoff or resonance changes. Do not freeze modulation to gain a cache hit.

## Acceptance and next boundaries

Reproduce the 180 controlled dry cases and 72 dynamic-filter cases specified in `behavior_fixtures.json`, against the comparison owner's recordings/reference. Historical maxima were 0.000130756 dB RMS / 0.005425825 dB spectral and 0.000325591 / 0.023800148 respectively, with fixed comparison gain 1. Those are history, not a claim for new code. Existing gates are RMS ≤ 1 dB and spectral ≤ 3 dB, with nonfinite 0. The reference owner supplies the exact metric and recordings; a different FFT or gain fitting is not an equivalent gate.

Then qualify two oscillators, morph/interpolation, bandlimit-on, FM/noise/sub, BP/HP, pitch tracking, tuning, two LFOs and routes, mono/legato/glide, allocator/damper/retrigger, arp/sequence, parameter automation, block partitioning, and all-off/reset/factory-switch tail ownership. The full FX path contains reduction of sample rate, equal-power autopan, phaser, filtered stereo delay paths, reverb with input/wet compression, master compression, and stereo widening. A generic Hall or shared global delay alone does not implement that graph.

Final admission still requires the unchanged 694-program gate, full supported controls and FX, two-part state isolation, complete native links with the existing 1024-byte XIP / 512-byte pool reserve floors, full callback deadline evidence, and listening approval. No result in this packet waives those gates.
