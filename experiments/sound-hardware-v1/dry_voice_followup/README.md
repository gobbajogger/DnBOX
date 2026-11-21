# Independent Synth One dry voice: 138/180 static notes, full gate FAIL

The independently owned mono voice combines existing DnBOX raw-saw and fastmath
components, the independently written S1 envelopes, and four forward poles
derived from the existing Chicago component. Original S1/SJ6 engines remain
comparison-only. The new voice is not connected to production. **Production
Synth One remains silent and unresolved.**

All 60 static files contain fresh notes 36/60/84 at 44,100 Hz. Comparisons use
the unchanged historical metric, explicit gain 1, RMS error at most 1 dB,
spectral error at most 3 dB, and zero nonfinite samples. No alignment,
normalization, fitted output gain or arbitrary tail retirement is used.

## Actual static results

| Frozen implementation | Passing notes | Passing files | Maximum RMS error | Maximum spectral error |
| --- | ---: | ---: | ---: | ---: |
| A: four poles, analytic coefficient, E=0 | 79/180 | 24/60 | 75.103715 dB | 20.987758 dB |
| B: two substeps, feedback slope mapping | 109/180 | 30/60 | 45.163240 dB | 15.629318 dB |
| C: centered implicit Chicago feedback | 109/180 | 30/60 | 7.691473 dB | 8.959005 dB |
| D: centered nominal-range linear feedback | 138/180 | 45/60 | 4.832778 dB | 8.991580 dB |

Every complete gate is **FAIL**, with zero nonfinite output. C removes B's
large sustained resonant tail but changes no passing-note count. D adds 29
passing notes relative to C and loses none. All 15 zero-resonance files remain
byte-identical between B, C and D, covering 45 notes. D's selected
`physical-gain/100_2000_.9` file now passes all three notes.

D's 42 failing notes are concrete remaining transfer differences:

| Cutoff | Resonance | Failing notes, at each velocity 20/100/127 | Worst RMS error | Worst spectral error |
| --- | ---: | --- | ---: | ---: |
| 64 Hz | 0.9 | 36 and 84 | 1.339043 dB | 4.622681 dB |
| 8,000 Hz | 0.5 | 36, 60 and 84 | 1.222203 dB | 2.361202 dB |
| 8,000 Hz | 0.9 | 36, 60 and 84 | 4.832778 dB | 2.817501 dB |
| 22,050 Hz | 0.5 | 36, 60 and 84 | 0.350263 dB | 3.987497 dB |
| 22,050 Hz | 0.9 | 36, 60 and 84 | 1.329834 dB | 8.991580 dB |

`evidence/c` and `evidence/d` retain actual run declarations, terminal commands,
all per-case reports and source review. `evidence/d/STATIC-ANALYSIS.json`
records the exact pass-set delta, worst cases and unchanged zero-resonance
hashes. Historical A/B evidence remains unchanged.

## Why C and D changed the filter

The independent analysis in `diagnosis/` derives B's actual delayed feedback
and its cutoff-dependent linear instability. C preserves the forward poles
and complete r=0 arithmetic while centering feedback with weights
(1,4,6,4,1)/16. It adds two history floats and solves its implicit Chicago
nonlinearity with six fixed iterations.

Eight isolated original-filter numeric observations and eight matched C
observations then exposed a different error. At a 2 kHz cutoff and sine,
resonance 0.9, original gain stays near +8.219749 dB across amplitudes 0.0001,
0.01 and 0.25. C's gain changes from +7.386123 to +5.411479 to -2.711459 dB.
Its compression is absent from those original records.

D makes only two semantic changes on the nonzero-resonance path:
`d=w/(1+5*c)` and feedback `5*d`. The slope five is the existing Chicago
primitive's derivative at zero; it is not fitted to an output. D retains C's
forward recurrence, coefficient preparation, two held-input substeps and
mean, gain, controls and histories. The entire r=0 branch and five other
renderer source files are unchanged. This eliminates the six-iteration solve
in favor of one division per substep. Native cost remains unmeasured.

`diagnosis/NONLINEAR-HYPOTHESIS.md` distinguishes frozen-coefficient linear
stability from arbitrary-modulation stability, and predicts a residual 2 kHz
transfer discrepancy. Removing saturation invalidates C's bounded-feedback
magnitude argument. Neither the theory nor the improved static counts proves
full original-filter equivalence.

## Ownership, scope and independent review

A/B use 152 host bytes per voice. C/D use 160: 92 prepared/control bytes and 68
history/flag bytes. Each owns phase, both envelopes and all filter history.
Physical cutoff 64–22,050 Hz, resonance 0–0.98 and depth 0–1.2 remain available.
Invalid control updates leave state unchanged. Retrigger remains explicitly
unsupported; no unqualified tail cutoff was added.

The focused C ownership follow-up and fresh D execution each compare 8,192
sample words, 310 complete state checkpoints and 310 untouched-owner checks.
Two differently configured owners execute independently and interleaved in
1/17/64-sample groups, with asymmetric note-offs and live physical edits.
All comparisons are exact and all 16,384 evaluated outputs finite. Each
logical owner is evaluated for 4,096 independent plus 4,096 interleaved frames.
The earlier C scalar-only ownership wording is narrowed explicitly in
`ownership_followup/FOLLOWUP.md`.

Independent D review verified the two semantic changes, the unchanged r=0
branch, six canonical renderer inputs, 15 source/support identities, three
binaries, eleven test-evidence files and all 33 frozen D evidence-manifest entries.
It found no blocker to measurement. This is source/test readiness, not an
audio pass. C's nonlinear scalar fixtures were not reused to qualify D's
different feedback law.

No complete 150-ID facade, 694-preset coverage, full oscillator/modulation/
allocator/FX graph, two-part instrument lifetime, native fit or callback
deadline is claimed. SuperJ6's separate 218-control/640-preset qualification
remains open. No original engine algorithm is an implementation dependency.

## Reference recovery and new observations

The exact archived static originals supply 60 files/180 notes. Six historical
zero-depth dynamic pairs have identical complete paired hashes to existing
static files, establishing 18 labelled aliases without new observations.
The 18 nonzero-depth historical paired files/54 notes remain missing.
`reference_followup/depth_zero_identity_aliases.json` records the exact mapping.
The original bounded search report is preserved; its omitted raw Library
listing is summarized by `bounded_library_lookup_summary.json` here.

A separate original-only rebuild completed 18 host compilations, one host
link and six audits. Its selected baseline produced exactly 1,058,304 bytes,
SHA-256 `b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2`,
matching the recovered three-note original column without repair or retry.
The new public wrapper and numeric/hash metadata are in
`reference_reconstruction`; original algorithms and source diagnostics remain
isolated in comparison recovery evidence. Matching this one baseline does
not authenticate every historical case.

One new original-only active recording uses the exact predeclared
`filter-modulation/1.2_2000_0.9` vector (filter ADSR 0.2/0.3/0.4/0.5,
velocity 100). Its 1,058,304 mono bytes hash to
`3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74`.
The active wrapper first reproduced D's static candidate byte-for-byte,
then rendered unchanged D once with the active controls. One host build,
one static replay, one active render and one unchanged gain-1 metric ran.
All three active notes passed, with zero nonfinite samples:

| Note | RMS error | Spectral error |
| ---: | ---: | ---: |
| 36 | 0.232227 dB | 0.682166 dB |
| 60 | 0.239112 dB | 0.767147 dB |
| 84 | 0.196105 dB | 2.106154 dB |

The candidate PCM SHA-256 is
`7ac88e50c0d0566c1a707dbeda8c81bed5b0848cfe44634011228999ea1f02b1`;
paired SHA-256 is
`338458922ef3861ec77898dfe386328a2ad6fa19897d265a010d106d96269bce`.
`active_filter_d/measured/` retains the exact run, controls, receipts and
scores. `reference_reconstruction/active_capture_proposal/` includes the
original numeric capture losslessly compressed, its encoding hashes, and
comparison-only provenance. The 3/3 active result is separate from the
138/180 static result, the historical 54 missing paired note cases, and
full-program qualification. No gain fitting, alignment or tolerance change
was applied.

### Reproduce the single active diagnostic

The active diagnostic uses frozen D and a separate host wrapper; it does not
change the DSP implementation. The preparation README, pre-review freeze and
exact historical command remain under `active_filter_d/`. The later actual
result is `active_filter_d/EXECUTION-REPORT.md`. To replay from a fresh checkout,
provide the original active PCM by decompressing the adjacent reference gzip
and provide D's saved selected static PCM from the recovery bundle (or from an
exact preceding full D run). The runner checks their hashes and the D180
summary. The output must be a fresh direct child of its own `results/` folder.
From `experiments/sound-hardware-v1/dry_voice_followup`:

```sh
python active_filter_d/check_active.py \
  --d-root variants/d --d-summary evidence/d/audio180-SUMMARY.json \
  --d-selected /absolute/path/100_2000_.9.candidate-mono.f32le \
  --new-original /absolute/path/filter-modulation_1.2_2000_r0.9.original-mono.f32le \
  --new-original-sha256 3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74 \
  --output active_filter_d/results/new-replay
```

The host GCC executable must match its declared hash. The runner stages only
verified bytes, refuses existing/symlink output paths and enforces static
identity before the single active render/metric. Fresh reproduction is not a
reason to overwrite or retry the recorded actual attempt.

## Portable reproduction

Recover `sound-pcm-preservation.tar` from the linked row in
[`docs/LIBRARY-INDEX.md`](../../../docs/LIBRARY-INDEX.md). Its 211,988,480 bytes
must hash to
`d5d11628a390f19d99783b1fb805979d371f5971efa01737c00bbc1e359b5403`.
From the repository root:

```sh
python experiments/sound-hardware-v1/dry_voice_followup/recover_references.py \
  --archive /absolute/path/sound-pcm-preservation.tar \
  --output /absolute/path/new-original-mono
python experiments/sound-hardware-v1/dry_voice_followup/check.py \
  --variant d --reference-root /absolute/path/new-original-mono \
  --output /absolute/path/new-d-screen --all
```

Recovery verifies the archive, compressed members, paired recordings and mono
hashes, then exports only the original column. It executes no original engine.
`--case physical-gain/100_2000_.9` narrows recovery to the selected file.

`check.py` is the supported comparison entry. It verifies the manifest, plan,
metric, source and reference hashes, stages verified in-memory snapshots into
a fresh output directory, builds the independent source and freezes the
new executable before comparison. Existing or symlinked outputs are rejected.
Mandatory checks stay active under optimized Python. Exit 1 is a measured
fidelity failure; exit 2 is incomplete/invalid execution and never a pass.

Omit `--all` for the selected three-note file. `--verify-only` checks inputs
without building/rendering. `--saved-candidates DIR` remeasures exact historical
A/B candidate PCM against the pinned old report bundle; it is not supplied
for C/D. No resume path exists. The selected optimized-Python B replay was
byte-identical in candidate PCM, paired PCM and metric JSON. Four regression
tests cover real provenance, coverage, threshold and overwrite defects.

Historical runners, absolute commands and pre-audio declarations are immutable
evidence, not the current execution interface. Lossless gzip copies preserve
logs and D's unified patch where their exact whitespace is significant;
`evidence/EXACT-LOG-ENCODING.json` records original and compressed hashes.
The final recovery bundle retains their original bytes as well.
