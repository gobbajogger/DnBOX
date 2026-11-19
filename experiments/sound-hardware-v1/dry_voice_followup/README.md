# Independent Synth One dry voice: measured progress, static gate FAIL

This package assembles an independently owned mono voice from existing DnBOX
raw-saw, fastmath, the new S1 envelope component and the existing Chicago
four-pole/nonlinear-feedback primitive. Original S1/SJ6 engines remain
comparison-only. This code is not connected to production; production Synth
One remains silent and unresolved.

The original reference packet supplies60 physical files, each containing three
fresh notes36/60/84 at44,100 Hz. Every comparison uses the unchanged historical
metric, explicit gain1, RMS error at most1 dB, spectral error at most3 dB and
zero nonfinite values. No alignment, normalization, fitted output gain or
arbitrary tail retirement is allowed.

| Frozen implementation | Passing notes | Passing files | Maximum RMS error | Maximum spectral error |
| --- | ---: | ---: | ---: | ---: |
| A: existing four poles, analytic coefficient, E=0 | 79/180 | 24/60 | 75.103715 dB | 20.987758 dB |
| B: two substeps, internal feedback slope mapping | 109/180 | 30/60 | 45.163240 dB | 15.629318 dB |

Both have zero nonfinite output and both fail the complete static gate. B
passes all45 zero-resonance notes across every supplied velocity, cutoff and
pitch. Its resonant failures include sustained output after note-off where the
reference has become silent. `HISTORICAL-REPORT.md`, `evidence/STRATIFIED.json`
and the complete compressed per-case reports preserve these failures.

## Implemented scope

Each152-byte host voice owns oscillator phase, amplitude/filter envelopes and
four-pole filter history. It applies component gain and squared velocity,
then the amplitude envelope before a common filter. Physical cutoff64–22050 Hz,
resonance0–0.98 and filter depth0–1.2 remain available. Invalid controls leave
state unchanged; retrigger is explicitly unsupported. The six envelope
coefficients in this frozen scene match the numeric reference contract.

No150-ID facade,694-preset coverage, full oscillator/modulation/allocator/FX
graph, two-part lifetime, native RAM/flash fit or callback deadline is claimed.
The existing original-source renderer is not used to implement this voice.

## Exact references and portable reproduction

Recover the exact `sound-pcm-preservation.tar` from the linked row in
[`docs/LIBRARY-INDEX.md`](../../../docs/LIBRARY-INDEX.md). Its211,988,480 bytes
must hash to
`d5d11628a390f19d99783b1fb805979d371f5971efa01737c00bbc1e359b5403`.
From the repository root:

```sh
python experiments/sound-hardware-v1/dry_voice_followup/recover_references.py \
  --archive /absolute/path/sound-pcm-preservation.tar \
  --output /absolute/path/new-original-mono
```

The recovery script exports only the original mono column and verifies the
archive, every compressed member, every paired recording and every mono hash.
It does not execute the original engine. `--case physical-gain/100_2000_.9`
performs a one-file recovery; omit it for all60.

The supported comparison entry is **`check.py`**. Historical `run_screen.py`,
`compare_saved.py`, declarations and reproduction notes remain immutable
evidence; their absolute paths and resume/assertion behavior are not the
current execution interface.

```sh
python experiments/sound-hardware-v1/dry_voice_followup/check.py \
  --variant b --reference-root /absolute/path/new-original-mono \
  --output /absolute/path/new-b-screen
```

This builds the frozen independent host source, verifies its API/scalar
invariants, freezes a new source/executable declaration, renders the selected
three-note file and invokes unchanged `metrics.py` with explicit gain1. It
accepts only a new output directory, including rejection of existing/dangling
symlinks and symlink parents. It uses verified in-memory source/metric snapshots
when staging the build and metric, so later edits to the live source cannot
change the declared experiment. All mandatory checks remain active under
optimized Python. Expected status for the selected A/B file is1: measured
audio failure. Status2 means incomplete/invalid execution and is never a PASS.

Add `--all` to compare all60 files/180 notes serially. `--verify-only` checks the
chosen source, plan, metric and reference recordings without compiling or
rendering. It does not verify candidate results. `--saved-candidates DIR`
remeasures exact retained candidate PCM, authenticated against the pinned
historical report bundle, without a build or audio render. No resume is used.

The selected B replay under `PYTHONOPTIMIZE=1` produced byte-identical candidate
PCM, paired PCM and metric JSON to the frozen B run; it retained the expected
2/3-note result and overall FAIL. `test_check.py` adds meaningful regression
cases for missing/duplicated/reordered notes, mismatched PCM/gain/nonfinite
counts, unchanged thresholds and output preservation. Tests pass in ordinary
and optimized Python.

## Independent review and next correction

The review checked all60 originals,120 candidate/paired outputs and their exact
columns,120 report/receipt/log sets, complete case/note counts, summaries and
551 archive-manifest files. Existing measured results are consistent. The
historical harness's four provenance/portability/overwrite weaknesses are
recorded in `evidence/historical_review`; the separate current checker fixes
them while preserving the original evidence.

`diagnosis/` derives B's actual two-substep feedback delay and its
cutoff-dependent instability. A single centered implicit-feedback proposal
preserves B's forward poles and zero-resonance audio while restoring the
linear stability boundary. Its proposal adds two floats and uses six bounded
iterations; theoretical stability does not establish original audio fidelity.
Its implementation and comparison are a separately declared next experiment,
not a third result in the table above.

The original static PCM cannot establish the entire dynamic/preset behavior.
Any recovered zero-depth aliases or new active-filter references must be
documented additively with exact hashes; old missing-input statements and
historical FAIL reports remain evidence of their original scope.
