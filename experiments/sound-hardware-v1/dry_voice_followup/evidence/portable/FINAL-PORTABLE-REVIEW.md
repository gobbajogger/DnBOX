# Final independent review: portable dry comparison entry

## Verdict

**No remaining blocking integrity, portability, or overwrite finding was identified in the reviewed A/B entry.** The early-validation/live-file gap found during review has been fixed and independently checked with compiler-free temporary-copy probes under normal Python and `-O`.

This verdict concerns the portable comparison harness. It is not audio-gate admission or target qualification. A remains 79/180 and B remains 109/180 in the preserved historical static screen; both gates remain FAIL. Variant C is not part of the manifest reviewed here and is outside this verdict.

Reviewed root: `/workspace/scratch/e803955aec60/DnBOX/experiments/sound-hardware-v1/dry_voice_followup`.

| Reviewed file | SHA256 |
| --- | --- |
| `check.py` | `f9908b030e7ed7efe6c3f3f412f47103fd85a205caf661f996826c051c7d2009` |
| `recover_references.py` | `d92e7edf314ff3f575a76464485a7cc52af5ebdd4e1efeb8ce55dd284f8f9a88` |
| `test_check.py` | `54e03eec37e5d9c85299e0c09f1cd9c92f5fee7ab2602c13a470bdd54dfe1752` |
| `FROZEN-INPUTS.json` | `a272b1120f1dd416ce521ab0d65391266b201efe5493681d859b771b40449785` |

All nine declared files per A/B variant match their expected hashes. The checker and recovery utility contain no Python `assert` statements.

## Required protections now present

**Portable, immutable input identities.** `check.py:62-98` resolves frozen files relative to the imported tree, validates the pinned manifest/plan/metric, and retains the exact validated bytes. The original absolute declarations remain historical files. The new code does not execute them or look up their old scratch paths. Selected references are length/hash/finite checked; the plan is pinned and contains exactly 60 unique physical cases.

**The bytes compiled and measured are the bytes validated.** `check.py:69-79,130-163` holds source bytes and stages only the six declared translation-unit/header inputs in the new build directory. Each staged source is hash checked. `check.py:185-189,240-242` stages the validated unchanged metric, checks its digest, and passes explicit comparison gain `1`. Changes to the original files after validation cannot enter those snapshots. The compiler path, compiler hash, version, flags, resulting executable hash, and host description are recorded; the new executable is deliberately distinguished from the historical executable hash.

**Fresh evidence and explicit failure status.** `check.py:47-59` uses exclusive metadata writes, rejects existing or dangling-symlink output paths, rejects symlink parents, and creates a fresh directory. The new flow never resumes on an old receipt. Only fresh output paths are supplied to the historical metric's ordinary JSON writer. `check.py:118-127` rejects nonzero child status, and `check.py:272-277` distinguishes comparison errors from a completed failed audio gate. There is no path by which a stale comparison error log is reused as completed evidence.

**Complete per-case reports and unchanged gate.** `check.py:101-115` verifies the paired size/hash, gain, exact ordered note triplet, actual nonfinite count, and finite nonnegative metric fields. `check.py:214-269` creates the pair from the verified original and current candidate, measures it, hashes the new report, derives every receipt/summary from those newly measured rows, and requires the expected row count. Whole-file acceptance still requires all three notes. Missing reference/candidate inputs stop the run; the checker does not silently repair coverage.

**Saved candidate mode remeasures instead of trusting editable numeric reports.** `check.py:192-198,217-222` verifies the historical report-bundle bytes against the pinned manifest and checks the saved candidate against its pinned historical receipt. It then runs the same fresh paired comparison. Numeric rows from historical summaries do not determine the new outcome.

**Bounded source-free recovery.** `recover_references.py:39-87` checks the exact archive size/hash, pinned plan, unique regular member, compressed member size/hash, decompressed paired size/hash, and extracted mono size/hash/finite status. It uses `extractfile` and derives fixed output basenames from the pinned case IDs; it does not extract arbitrary archive paths or read engine implementation. Outputs and the recovery receipt are exclusive, beneath a new directory. The recovered candidate column is not exported.

## Review finding resolved during this pass

The earlier checker hashed live source and metric paths, then reread them later for compilation or execution. That permitted the declared identity to differ from the bytes subsequently used. Root changed the interface to retain validated source and metric bytes and stage those snapshots. The reviewer did not edit canonical files.

The bounded probes in `portable_probes.py` made only temporary copies and intercepted the first compiler/metric call before any subprocess execution:

| Probe | Normal Python | `python -O` |
| --- | --- | --- |
| Validate the relocated B source and selected reference | PASS | PASS |
| Change the relocated source before validation | Rejected: frozen source changed | Same rejection |
| Change B's output multiplier after validation | Staged source stays byte-identical to the validated frozen source | Same result |
| Change the source metric after validation | Staged metric remains the pinned historical bytes; invocation gain is `1` | Same result |
| Original canonical source and metric preserved | Yes | Yes |

The probes executed zero compiler, renderer, and metric subprocesses. Their first attempt overlapped root's interface change and stopped with a review-script argument-type error; that was not a product failure. The probes were updated for the new interface, and both completed successfully. `PORTABLE-PROBES.json` contains the completed receipts.

## Saved replay corroboration

Without running any audio or metric, I compared the existing root-provided B replay at `/workspace/scratch/e803955aec60/dry_portable_validation/b_replay` with the preserved historical B case `physical-gain/100_2000_.9`.

| Artifact | Byte equality | SHA256 |
| --- | --- | --- |
| Candidate mono, 1,058,304 bytes | Exact | `f440fbb92f1fd2f6fef70068127cfd6f02b164c8927f90117ba28e3a003cb8a6` |
| Paired PCM, 2,116,608 bytes | Exact | `40002ee7d0c10a5bdd4d37e6d7ccaf37048a614e3affa28d792489205c575000` |
| Metric JSON, 526 bytes | Exact | `0b7e16de8422e42b54d08c458da54719afbb460d7bd35207f7d568f015c1541f` |

Its new summary correctly records one complete file, three notes, two passing notes, zero passing files, and `accepted: false`. Maximum RMS is 1.9348981746235328 dB and maximum spectral error is 3.099491563393029 dB. No source snapshot change affects the DSP bytes used for that replay.

## Nonblocking scope notes

- `--verify-only` returns at `check.py:180-183`, before validating `--saved-candidates` or its historical report bundle. Its success means the selected frozen source/plan/metric/reference inputs were verified. Document that scope, or reject/clarify the combination with `--saved-candidates`; do not present it as validation of saved candidate outputs.
- The tests directly exercise the former report-coverage, gain/hash/nonfinite, threshold, and output-preservation defects. Root reported four passes under normal Python and `-O`; I reviewed those tests and did not repeat them. The new source/metric snapshot probes address the one new risk uncovered here. Further broad sweeps or optional rerenders are not needed for this review.
- Adding C requires updating both the frozen manifest and the checker's manifest digest, with C's exact files, expected host size, and historical evidence identity. That subsequent manifest and C's DSP behavior have not been reviewed in this receipt.

The separate checker/recovery entry is appropriately bounded. It preserves historical evidence and the fixed metric without introducing a new engine, new thresholds, fitted gain, alignment, arbitrary retirement, full-suite work, or target qualification. All review outputs are in the reviewer's directory; no canonical source was changed by the reviewer.
