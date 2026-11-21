# Independent review of the bounded dry-voice harness

## Result

**The existing saved evidence is internally consistent. Both audio gates remain FAIL.** The historical reproduction scripts are not suitable as portable, fail-closed verification after import. Preserve them and their declarations unchanged; add a separate checker rooted in the imported files.

This is an independent read-only review, not gate admission. No original S1/SJ6 DSP implementation, tables, source-aware candidate, renderer, compiler, native target, emulator, full suite, or RDC was used. The only new files are this review's artifacts. The original runner, summarizer, and comparison utility were not executed. Recorded RMS/spectral values were reconciled against their saved reports/logs and manifest; this review did not recompute FFT metrics.

The review root is `/workspace/scratch/e803955aec60/s1_dry_voice_unit`; the source-free oracle packet is `/workspace/scratch/e803955aec60/dry_reference_packet`.

## Observed artifact checks

| Check | Observed result |
| --- | --- |
| Recovered reference files | 60/60 have the specified SHA256, 1,058,304 bytes, and finite samples |
| Frozen candidate/utility inputs | All 13 entries per declaration match, at their historical absolute locations |
| Candidate and paired outputs | 120/120 candidate hashes and paired hashes match their recorded receipt/report |
| Paired PCM columns | Each reference column is bit-identical to its current verified mono reference; each candidate column is bit-identical to its candidate mono file |
| Coverage | Exactly 60 unique physical cases and notes `[36,60,84]` for each variant; 180 distinct case/note pairs each |
| Gain and finite status | All 120 reports/receipts use gain 1; all saved paired samples are finite |
| Logs | All 120 render logs show exit 0 and the expected command/stimulus; all 120 compare logs contain the exact saved report and receipt |
| Failed comparisons | A has 36 exit-1 logs and B has 30; each is a normal completed failed-gate comparison, with valid report and receipt |
| Bundle manifest | All 551 listed files match their size and SHA256 |
| Symlinks | None observed in the two input trees |

| Variant | Passing notes | Passing files | Maximum RMS dB | Maximum spectral dB | Zero-resonance notes |
| --- | ---: | ---: | ---: | ---: | ---: |
| A | 79/180 | 24/60 | 75.10371468144244 | 20.987757969618144 | 27/45 |
| B | 109/180 | 30/60 | 45.16323956281096 | 15.629317504494594 | 45/45 |

No current mismatched comparison log, missing case, altered report, paired-column substitution, or false audio PASS was found. Per-file hashes and all reconciliation results are in `machine_receipt.json`; the read-only audit implementation is `audit_saved.py`.

## Required fixes before portable verification

### R1 — P1: verification reads old scratch files instead of imported files

Evidence: `/workspace/scratch/e803955aec60/s1_dry_voice_unit/DECLARATION.json:30-43` and `variant_b/DECLARATION.json:30-43` pin absolute scratch paths. `run_screen.py:29-32` and `variant_b/run_screen.py:29-32` hash those paths directly. `summarize.py:52-58` does the same for inputs and references. `replay.py:20-30` relocates the executable/runner but leaves the declaration's hash lookups absolute. `REPRODUCE.md:3-9,16-21` also assumes the historical workspace.

After import under `experiments/sound-hardware-v1/dry_voice_followup`, verification either fails when that old workspace is absent or checks old source/binary files while executing the newly imported binary. Thus an imported binary can differ from the one whose hash passed. `summarize.py:68` additionally treats current filesystem mtime as historical declaration ordering; extraction/copy/git timestamps are not a portable chronology record.

Minimal fix: preserve original declarations and their hashes as historical evidence. In a separate checker, explicitly map the two historical roots to the current candidate and packet roots, reject unmapped/escaping paths, and hash the actual imported files. Store the mapping and resolved current relative paths in a new receipt. Treat declaration-before-render as a retained historical assertion with its original evidence, not a new proof derived from copy mtimes. Keep the saved binary identity distinct from a later compiler's rebuilt binary identity.

### R2 — P1: optimized Python removes integrity checks

Evidence: `run_screen.py:34-48,60,81-83` and the corresponding variant B lines use `assert` for plan, selection, reference, gain, and candidate/reference identity checks. `summarize.py:53,57,66-71` uses `assert` for every material integrity condition before emitting `PASS_HASH_INTEGRITY_ONLY` at line 72.

Compiling these exact files in memory with `optimize=1`, without executing them, removes all 13 assertion operations from each runner and all 7 from the summarizer. The summarizer can then increment its checked counters and emit PASS without actually reading/hashing candidate PCM or references in the removed expressions. A changed `PYTHONOPTIMIZE` environment can therefore change the meaning of verification.

Minimal fix: explicit validation branches raising a defined verification error. The new checker must produce the same result under ordinary Python, `python -O`, and `PYTHONOPTIMIZE=1`. Do not rely only on refusing `-O`; fail-closed checks should survive optimization.

### R3 — P1: resumed reports are not bound to complete measured results

Evidence: `run_screen.py:61` treats an existing receipt as completed work and bypasses render/comparison. Lines 79-89 trust the saved metric rows, validate only candidate/reference hashes and gain, and never verify `report.pcm_sha256`, receipt case/metric identity, the `[36,60,84]` triplet, or the compare log. Line 107 accepts when every *present* row passes. `summarize.py:28-50` trusts the aggregate summary, while lines 60-71 check PCM hashes without reconciling its numeric rows, receipt acceptance, or log status.

A missing failing note can yield a successful return for an incomplete case. Editing a report's numeric rows while retaining its `pcm_sha256` is not detected by these hash checks. An extant receipt also permits a stale or mismatch/error compare log to be ignored on resume. These are latent risks; the actual 120 logs/reports all reconcile.

Minimal fix: require the exact case/note set, validate all identity fields, recompute acceptance from all rows, reconcile report/receipt/log/summary, and require completed valid evidence independently of file existence. Distinguish completed failed-gate evidence from incomplete or invalid evidence in both status and exit behavior. A normal compare exit 1 is valid only when its complete report and receipt match the failed metric outcome; a reference mismatch/traceback is invalid evidence even if an older receipt exists.

For this historical set, an independently pinned bundle-manifest identity can authenticate the saved report bytes. To certify the numerical metrics against PCM independently, use the unchanged metric on copies of the **120 already saved paired PCM files** in a fresh temporary output tree with explicit gain `1`, then compare its results to the saved reports. That is a bounded remeasurement of saved data, with no synthesizer/audio rerender. Original PCM, reports, logs, and declarations must remain untouched. Do not claim a hash-only reconciliation independently recomputed the metrics.

### R4 — P2: the no-overwrite promise does not cover metadata and dangling symlinks

Evidence: `/workspace/scratch/e803955aec60/dry_reference_packet/compare_saved.py:50-53` checks only paired output and report `Path.exists()`. The paired data creation is correctly exclusive at lines 82-83. However, the receipt at line 103 is written with ordinary `write_text` without any existence check, and the immutable `metrics.py:11` uses ordinary `write_text` for the report. A dangling report symlink is invisible to `exists()` and can be followed by that write; any existing receipt or receipt symlink can be overwritten. `run_screen.py:66,74,104-105` and `summarize.py:84-94` also replace log/summary files. `REPRODUCE.md:76` states broadly that the renderer and utility refuse to overwrite their outputs.

The C renderer's `src/host.c:136` (`wbx`) and paired PCM's `xb` are correct for exclusive raw-file creation. No symlink or overwrite incident was found in saved data.

Minimal fix: leave the historical metric unchanged. A new wrapper should use a newly created owned output directory, reject preexisting metadata paths including dangling links (`lexists`/`lstat`), confine resolved paths to that directory, and create its own receipt exclusively. Give derived checks/summaries new output names/directories instead of replacing historical evidence. State the historical utility's limits in the portable instructions.

## Minimum portable checker contract

1. **Current roots and immutable identities.** Explicit candidate and packet roots; current paths confined to those roots; original declarations preserved; metric SHA256 exactly `bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1`; current input hashes checked using mapped paths. No lookup of historical implementation dependencies is required or permitted by this review.
2. **Stimulus.** Sample rate 44,100; warm-up 176,448; recorded frames per note 88,192; note-off frame 66,112; note order `[36,60,84]`; frequency bits `[4282d013,4382d013,4482d013]`; three velocities × five cutoffs × four resonances; static filter depth 0. Require precisely the 60 physical cases from the pinned plan, with unique IDs and unique case/note pairs.
3. **PCM and hashes.** Mono size 1,058,304; paired size 2,116,608; little-endian float32; reference SHA equals the plan/manifest; candidate SHA equals receipt; paired SHA equals report; paired columns equal their respective mono files bit-for-bit; actual nonfinite counts equal recorded counts.
4. **Report schema.** Exactly three unique ordered notes; gain exactly 1; finite, nonnegative RMS/spectral errors; integer nonfinite count; valid SHA fields; per-note acceptance is RMS ≤1 dB, spectrum ≤3 dB, and nonfinite=0. Accept a file only if every required note passes.
5. **Receipt and completion.** Exact case ID; `verified-mono` reference layout; exact reference/historical paired/candidate/metric hashes; comparison gain 1; `accepted` equals recomputed acceptance. The log's exit/status and two emitted JSON documents must agree with report/receipt. Unexpected stderr/mismatch/traceback cannot be completed evidence. The renderer log must contain the expected physical parameters and fixed stimulus metadata.
6. **Aggregation.** Rebuild the summary from verified per-case reports. Require 60 cases and 180 notes before emitting complete status; reconcile every saved row, pass count, maxima, and nonfinite count. Keep audio FAIL separate from evidence integrity status.
7. **Saved vs new claims.** Hash-only checks, metric remeasurement, new rendering, host selftests, and target qualification are distinct receipt fields. This review performed the first of those plus PCM/log consistency checks. It grants none of the others.

## Small mutation checks for the new checker

Use a temporary copy of the imported data; never mutate the historical tree or run its original runner. A copy-on-write filesystem copy is sufficient. Each invalid fixture must be rejected with an explicit integrity/coverage error under normal Python **and `-O`**, without invoking a compiler or audio renderer. Preserve the baseline distinction: complete evidence with audio FAIL is different from invalid evidence.

| Negative | Exact mutation in the temporary copy | Required result |
| --- | --- | --- |
| Report tamper | In B `results/ch4_2x_slope4/100_2000_.9.paired.json`, change note 84 RMS/spectral values to `0.0` while leaving `pcm_sha256` unchanged | Report authentication or metric reconciliation fails; no acceptance of the fabricated pass |
| Missing note | Remove note 84 from that report; also remove its aggregate row if testing beyond a simple summary mismatch | Exact-note coverage fails even though the two remaining notes pass |
| Missing case | Remove one case's receipt (or all its six artifacts) and the corresponding summary rows | Exact 60-case/180-note coverage fails; the checker never renders a replacement |
| Mismatch log with old receipt | Replace that case's compare log with exit 2 plus `reference hash mismatch`; retain the prior receipt/report | Invalid completion evidence; receipt existence does not make it complete |
| Optimized interpreter | Recheck the baseline and the four mutated copies using `python -O` and/or `PYTHONOPTIMIZE=1` | Same verification decisions as ordinary Python; no assertion-dependent PASS |

For a stronger report-tamper check, update the temporary receipt, summary, and log to agree with the fabricated zero metrics. The checker must still reject against a trusted report-manifest identity or independent unchanged-metric remeasurement. This prevents merely mutually consistent edited metadata from certifying itself.

## Source and selftest assessment

The reviewed C implementation follows its declared bounded experiment: per-instance mutable state, fixed 44.1 kHz, explicit raw saw phase, squared velocity, unchanged local envelope component, experimental four-stage filter, and variant B's two substeps/0.8 internal feedback factor. The two variants' headers/host sources are identical; their DSP differences are limited to the declared filter changes. `src/host.c:138-160` implements the recorded note order, warm-up, note-off frame, output length, and fixed gain metadata.

The saved selftests report PASS for ownership, rejected state changes, physical endpoints, squared velocity, unsupported retrigger, and an event at a 64-sample boundary (`src/host.c:43-104`). Their custom `CHECK` macro is independent of Python optimization. These are API invariants; they do not establish original-engine equivalence, arbitrary sub-block event scheduling, dynamic-filter fidelity, presets, target size, or deadlines. No selftest was rerun during this review, and no C defect was found that changes the interpretation of the recorded static failures.
