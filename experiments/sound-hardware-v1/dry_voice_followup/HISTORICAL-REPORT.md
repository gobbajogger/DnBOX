# Independent Synth One dry-voice prototype: exact static gate remains FAIL

Two bounded prototypes were implemented using the approved existing DnBOX raw
saw, fast math, S1 envelope component, and Chicago four-stage filter/feedback
primitive. Both were declared before their first audio render. Both completed
the recovered 60 physical files / 180 note cases using the unchanged comparison
metric, gain 1, no alignment and no normalization. Neither passes the full static
dry gate. No production source or activation was changed.

Variant B provides a useful narrower result: **all 45 zero-resonance note cases
pass**, covering every supplied velocity, cutoff and pitch. Resonant nonlinear
transfer remains a concrete blocker, including sustained candidate feedback
oscillation at settings where the recorded reference has ended its tail.

## Frozen gate and outcomes

Each note must have RMS error at most 1 dB, spectral error at most 3 dB, and zero
nonfinite samples. One file contains independent notes 36, 60 and 84; a file
passes only when all three notes pass. These are the existing gate and metric,
not newly selected thresholds.

| Variant | Passing notes | Passing three-note files | Maximum RMS error | Maximum spectral error | Nonfinite |
| --- | ---: | ---: | ---: | ---: | ---: |
| A: stock four-stage recurrence, analytic pole, E=0 | 79/180 | 24/60 | 75.103715 dB | 20.987758 dB | 0 |
| B: two filter substeps, analytic feedback slope correction | 109/180 | 30/60 | 45.163240 dB | 15.629318 dB | 0 |

The first required file was `physical-gain/100_2000_.9`, whose original-mono hash
is `b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2`.

| Note | A RMS error | A spectral error | B RMS error | B spectral error |
| ---: | ---: | ---: | ---: | ---: |
| 36 | 6.865833 dB | 4.010972 dB | 0.139213 dB | 2.291580 dB |
| 60 | 4.928747 dB | 4.780310 dB | 0.881050 dB | 2.950266 dB |
| 84 | 1.733724 dB | 4.104239 dB | 1.934898 dB | 3.099492 dB |

Variant A fails all three selected notes. Variant B passes notes 36 and 60 but
fails note 84 on both limits. The file therefore remains FAIL for both variants.

## What was built

The unit is a fixed-rate, independently owned mono voice with typed physical
controls. It performs the following operations in order:

1. Advance one amplitude envelope and one independently owned filter envelope
   using the unchanged DnBOX S1 component.
2. Produce the stock raw saw, `2*phase - 1`, with explicit phase initialized to
   zero and the exact supplied binary32 note frequency. No BLEP is enabled.
3. Apply oscillator component gain `(1-balance)*level`, squared MIDI velocity,
   and then the amplitude envelope before the filter.
4. Compute the physical cutoff law
   `clamp(C*(1-D*(1-E)),64,22050)` and prepare a changed filter coefficient.
5. Advance one common four-stage nonlinear lowpass and output mono directly.

There is no division by the number of voices, center-pan half-gain, master gain,
comparison gain, time shift or output normalization. Direct copying of this mono
output into left and right is a future adapter responsibility.

The typed adapter accepts the complete requested physical cutoff, resonance and
depth domains: 64–22050 Hz, 0–0.98 and 0–1.2. Invalid/nonfinite values leave its
state unchanged. The unit exposes no raw-ID storage facade and makes no claim
to implement the 150-parameter namespace. A fresh note is supported; retrigger
after a note has started returns an explicit unsupported result. Release is not
arbitrarily truncated and does not declare a note retired.

The host-verified object is **152 bytes**, including 92 bytes of physical/prepared
parameters and 60 bytes of history, oscillator state and flags. Each owner has
its own complete state. There are no mutable globals, dynamic allocations or
libm calls in the candidate. This is a host object size, not native RAM or flash
fit evidence. The production owner may later share immutable control data while
retaining independent per-note caches and histories.

## Source boundary and filter choices

Only the approved DnBOX components and source-free numeric packet were used.
No original S1/SJ6 engine source, original tables/assets, source-aware candidate,
reference-owner build source or historical owner-evidence implementation was
read or copied. The approved production Chicago component was inspected at
`DnBOX/experiments/original_synth/chicago_mp4.c`; only its four sequential poles
and feedback nonlinearity were extracted. Its tables, oscillator path, kick,
ramps, coefficient reader, stereo gain and resonance postgain are absent.

Variant A prepares `A=fm_expf(-2*pi*cutoff/44100)`, holds the Chicago allpass
coefficient E at zero, and feeds physical resonance directly to the existing
feedback primitive. E=0 retains the stock extra feedback history. This was an
explicit independent transfer experiment because the original Chicago
coefficient tables were neither allowed nor needed for the experiment.

After A's measured failure, B was separately frozen with one stated hypothesis:
the stock feedback polynomial has small-signal slope five, while the continuous
model of four equal lowpass poles has critical loop gain four. B therefore uses
`R=physical_resonance*(4/5)` internally. It also runs the unchanged pole recurrence
at two fixed substeps per output sample, preparing its pole at 88200 Hz, holding
the input for both substeps, and averaging the two outputs. This addresses the
measured high-frequency discrete transfer error and feedback behavior. The
oscillator, envelope timing, stored physical controls and output gain are
unchanged. These were analytic structural choices, not a coefficient sweep or
an output-gain fit. No third variant was made.

Both versions reuse `dnbx_s1_envelope.c` without changes. All six retention
coefficients used in this exact dry packet match the numeric contract's
double-prepared/binary32-rounded values exactly. They are prepared outside the
sample loop using stock `fm_expf`. This work did not rerun the previously
reported 379288 envelope observations or qualify arbitrary physical coefficient
values. The six checked words are `3f7fce7c`, `3f7ed775`, `3f7fe249`,
`3f7fce7c`, `3f7fe249`, `3f7ffd07`.

## Stratified result

| Resonance | A passing notes | A max RMS / spectral | B passing notes | B max RMS / spectral |
| ---: | ---: | ---: | ---: | ---: |
| 0 | 27/45 | 0.313955 / 7.672903 dB | 45/45 | 0.313897 / 2.283731 dB |
| 0.25 | 27/45 | 19.805560 / 9.259972 dB | 32/45 | 2.233194 / 2.533217 dB |
| 0.5 | 24/45 | 46.094877 / 17.275529 dB | 18/45 | 12.887401 / 8.447839 dB |
| 0.9 | 1/45 | 75.103715 / 20.987758 dB | 14/45 | 45.163240 / 15.629318 dB |

The following cells count passing notes out of the same three pitches.

| Velocity | Cutoff Hz | A, R=0 | A, R=0.9 | B, R=0 | B, R=0.9 |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 20 | 64 | 3/3 | 0/3 | 3/3 | 1/3 |
| 20 | 500 | 3/3 | 0/3 | 3/3 | 3/3 |
| 20 | 2000 | 3/3 | 0/3 | 3/3 | 3/3 |
| 20 | 8000 | 0/3 | 0/3 | 3/3 | 0/3 |
| 20 | 22050 | 0/3 | 0/3 | 3/3 | 0/3 |
| 100 | 64 | 3/3 | 0/3 | 3/3 | 1/3 |
| 100 | 500 | 3/3 | 0/3 | 3/3 | 2/3 |
| 100 | 2000 | 3/3 | 0/3 | 3/3 | 2/3 |
| 100 | 8000 | 0/3 | 0/3 | 3/3 | 0/3 |
| 100 | 22050 | 0/3 | 0/3 | 3/3 | 0/3 |
| 127 | 64 | 3/3 | 0/3 | 3/3 | 1/3 |
| 127 | 500 | 3/3 | 1/3 | 3/3 | 1/3 |
| 127 | 2000 | 3/3 | 0/3 | 3/3 | 0/3 |
| 127 | 8000 | 0/3 | 0/3 | 3/3 | 0/3 |
| 127 | 22050 | 0/3 | 0/3 | 3/3 | 0/3 |

The zero-resonance results support a specific improvement: B closes the
high-cutoff spectral failures of A in the supplied static grid. Its zero-
resonance RMS and spectral results are within the gate at all three velocities.
That is evidence for this narrow assembled path, not a proof that oscillator
phase, lifetime, arbitrary controls or full Synth One behavior are exact.

The resonance result is not corrected by a global gain. B changes from 46/60
passing notes at velocity 20 to 37/60 at velocity 100 and 26/60 at velocity 127;
the cutoff/resonance pattern also varies. At R=0.25, every spectral result is
within 3 dB while 13 notes fail the 1 dB RMS condition. At higher resonance there
are both transfer and level-dependent failures.

The clearest B failure is `physical-gain/20_22050_.9`. Note 36 has reference RMS
0.001194389 and candidate RMS 0.216425221, yielding +45.163240 dB. Note 84 has the
maximum spectral error, 15.629318 dB. In the last 4096 frames, 0.4078–0.5007
seconds after note-off, the stored reference is zero while B still has RMS
0.216439–0.216464. This is sustained feedback oscillation in the candidate,
not a small constant calibration error. The reference's voice-retirement rule
is not qualified by this unit; no cutoff of the candidate tail was introduced
to hide the discrepancy. With the same velocity and cutoff at R=0, B passes and
its last-window RMS is about 8e-7, consistent with its intentionally untruncated
envelope release.

A's worst RMS error is `physical-gain/20_64_.9`, note 84, 75.103715 dB; its worst
spectral error is `physical-gain/127_8000_.9`, note 84, 20.987758 dB. Those failed
outputs remain intact too.

## Validation and provenance

Only bounded host C builds and serial renders ran. Build flags are in `build.sh`
and `variant_b/build.sh`: C11, O2, no fast math, no FMA contraction, warnings as
errors, and no libm link. Compiler and undefined-symbol logs are preserved.

Focused tests pass for finite physical validation, nonmutating invalid input,
the full cutoff/depth endpoints, squared velocity, independently mutable owners,
block partitioning, the exact note-off event boundary, and explicit unsupported
retrigger. These are candidate invariants, not substitutes for the audio gate.

Each note uses the packet's exact frequency, 176448 silent warm-up frames,
note-on at frame 0, note-off before output frame 66112, and 88192 recorded frames
at 44100 Hz. The unit assumes already settled typed controls. It does not
implement the full engine's parameter smoothing.

`results/INTEGRITY.json` verifies that both declarations precede all their
candidate PCM files; all frozen source/binary/utility hashes still match; all
60 original mono hashes match; and all 60 candidate plus paired-PCM hashes match
for each variant. Metric SHA256 is
`bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1`.

| Evidence | Location |
| --- | --- |
| Variant A source and declaration | `src/`, `vendor/`, `DECLARATION.json` |
| Variant B source and declaration | `variant_b/src/`, `variant_b/vendor/`, `variant_b/DECLARATION.json` |
| Exact A 180-note rows | `results/ch4_exp_e0/all180_summary.json` |
| Exact B 180-note rows | `variant_b/results/ch4_2x_slope4/all180_summary.json` |
| Grouped comparisons | `results/STRATIFIED.json`, `results/PASS-GRID.md` |
| Raw level/tail diagnostics | `results/diagnostics.json` |
| Host API tests | `results/selftest.json`, `variant_b/results/selftest.json` |
| Per-case comparison receipts/logs/PCMs | Each variant's results directory |
| New-output reproduction | `REPRODUCE.md`, `replay.py` |

## Exact unresolved transfer

The assembly, state ownership, frozen ADSR preparation and zero-resonance static
path are reviewable progress. The current inherited nonlinear feedback is not
an accepted S1 resonant lowpass. A further investigation needs a separately
justified independent resonance/filter design and the unchanged static gate;
it should preserve these two failures and avoid disguising them with fitted
gain, a narrowed cutoff/resonance range or arbitrary tail retirement.

The 72 dynamic-filter cases cannot be claimed because their reference PCM is
not in the recovered packet. There is zero intact coverage of the 694-preset
gate. Two oscillators, morphing, enabled bandlimit, FM/sub/noise, BP/HP, modulation,
allocator/legato/glide/damper/retrigger, smoothing, controls/preset ingestion,
FX/master, two-part lifecycle, native fit and callback deadlines remain outside
this experiment. No production admission or activation is justified.
